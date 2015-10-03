/* OCaml bindings for the Xlib library.
   Copyright (C) 2008, 2009, 2010 by Florent Monnier
   printf("fmonnier@%s", "linux-nantes.org");
 
  OCaml-Xlib is FLOSS software:
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  The Software is provided "AS IS", without warranty of any kind, express or
  implied, including but not limited to the warranties of merchantability,
  fitness for a particular purpose and noninfringement. In no event shall
  the authors or copyright holders be liable for any claim, damages or other
  liability, whether in an action of contract, tort or otherwise, arising
  from, out of or in connection with the Software or the use or other dealings
  in the Software.
*/

// {{{ Headers 

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAML_NAME_SPACE 1

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/callback.h>

// }}}

#include "wrap_xlib.h"

// {{{ caml allocs 

custom_ops(XEvent);
custom_ops(XColor);
custom_ops(XGCValues);
custom_ops(XSetWindowAttributes);
custom_ops(XWindowAttributes);
custom_ops(XSizeHints);
custom_ops(XVisualInfo);
custom_ops(XChar2b);
custom_ops_n(XChar2b);

// }}}
// {{{ XID's 

#define Drawable_val(v) (XID_val(Drawable,(v)))
#define Val_Drawable(xid) (Val_XID((xid)))

#define Cursor_val(v) (XID_val(Cursor,(v)))
#define Val_Cursor(xid) (Val_XID((xid)))

#define Colormap_val(v) (XID_val(Colormap,(v)))
#define Val_Colormap(xid) (Val_XID((xid)))

#define GContext_val(v) (XID_val(GContext,(v)))
#define Val_GContext(xid) (Val_XID((xid)))

//#define KeySym_val(v) (XID_val(KeySym,(v)))
//#define Val_KeySym(xid) (Val_XID((xid)))
/* Get keysyms values instead of an abstract type */
#define Val_keysym Val_int
#define Keysym_val Long_val

// }}}
// {{{ return status 

#define DO_CHECK_RETURN_STATUS 1
#if DO_CHECK_RETURN_STATUS

#define GET_STATUS  int status =
#define CHECK_STATUS(fun,st) \
    do{ if (status != st) caml_failwith(#fun ": wrong return status"); }while(0)

#else

#define GET_STATUS (void)
#define CHECK_STATUS

#endif

/* Many GET/CHECK STATUS are commented just because in
 * the source code of the Xlib implementation I use (XOrg)
 * the return status is just static and can only return one
 * value, so testing it is meaningless.
 * They are kept commented in case they are meaningfull in
 * an other implementation.
 *
 * Sometime the return value meaning a success is 1 and sometime 0,
 * even in the commented GET/CHECK_STATUS the value is the good one.
 */

// }}}
// {{{ GC 

// Handle the finalisation of GC's (other than the default one)

#define GC_val(gc_pair) ((GC) Field((gc_pair),0))

#define Display_of_GC(gc_pair) (Field((gc_pair),1))

void Finalize_GC( value gc )
{
    value dpy = Display_of_GC(gc);
    if (display_is_open(dpy)) {
        GET_STATUS  XFreeGC(     
            Display_val(dpy),
            GC_val(gc)
        );
        CHECK_STATUS(XFreeGC,1);
    }
}
CAMLprim value do_finalize_GC( value gc ) {
    Finalize_GC( gc );
    return Val_unit;
}

/*
static struct custom_operations fgc_custom_ops = {
    identifier: "GC handling",
    finalize:  Finalize_GC,
    compare:     custom_compare_default,
    hash:        custom_hash_default,
    serialize:   custom_serialize_default,
    deserialize: custom_deserialize_default
};
*/

// The finalised one
static inline value Val_GC_final(GC gc, value dpy)
{
    CAMLparam1( dpy );
    CAMLlocal1( fgc );
    //fgc = caml_alloc_custom( &fgc_custom_ops, 2 * sizeof(value), 0, 1);
      // the previous line segfaults
      // so the link is done with do_finalize_GC from the ocaml side
      fgc = caml_alloc(2, 0);
    Store_field( fgc, 0, ((value)(gc)) );
    Store_field( fgc, 1, dpy );
    CAMLreturn( fgc );
}

// Not finalised (for default gc)
static inline value Val_GC(GC gc, value dpy)
{
    CAMLparam1( dpy );
    CAMLlocal1( gcp );
    gcp = caml_alloc(2, 0);
    Store_field( gcp, 0, ((value)(gc)) );
    Store_field( gcp, 1, dpy );
    CAMLreturn( gcp );
}

// }}}
// {{{ Event-Masks 

static const long event_mask_table[] = {
    KeyPressMask,
    KeyReleaseMask,
    ButtonPressMask,
    ButtonReleaseMask,
    EnterWindowMask,
    LeaveWindowMask,
    PointerMotionMask,
    PointerMotionHintMask,
    Button1MotionMask,
    Button2MotionMask,
    Button3MotionMask,
    Button4MotionMask,
    Button5MotionMask,
    ButtonMotionMask,
    KeymapStateMask,
    ExposureMask,
    VisibilityChangeMask,
    StructureNotifyMask,
    ResizeRedirectMask,
    SubstructureNotifyMask,
    SubstructureRedirectMask,
    FocusChangeMask,
    PropertyChangeMask,
    ColormapChangeMask,
    OwnerGrabButtonMask,
};

static inline long
Eventmask_val( value em_list )
{
    long event_mask = 0;
    while ( em_list != Val_emptylist )
    {
        value head = Field(em_list, 0);
        long mask = event_mask_table[Long_val(head)];
        event_mask |= mask;
        em_list = Field(em_list, 1);
    }
    return event_mask;
}

static inline value Focus_state_val(value mode) {
    switch Int_val(mode) {
        case 0:     return RevertToParent;
        case 1:     return RevertToPointerRoot;
        case 2:     return RevertToNone;
        default:    caml_failwith("Unknown focus state value");
    }
}

// }}}
// {{{ macro/funcs 

/* switch this to use the macros or the functions */
//#define XMP(f) X##f  // use the functions
#define XMP(f) f   // use the macros


#define _xConnectionNumber  XMP(ConnectionNumber)
#define _xRootWindow        XMP(RootWindow)
#define _xDefaultScreen     XMP(DefaultScreen)
#define _xDefaultRootWindow XMP(DefaultRootWindow)
#define _xDefaultVisual     XMP(DefaultVisual)
#define _xDefaultGC         XMP(DefaultGC)
#define _xBlackPixel        XMP(BlackPixel)
#define _xWhitePixel        XMP(WhitePixel)

#define _xDisplayWidth      XMP(DisplayWidth)
#define _xDisplayHeight     XMP(DisplayHeight)

#define _xDisplayPlanes     XMP(DisplayPlanes)
#define _xDisplayCells      XMP(DisplayCells)
#define _xScreenCount       XMP(ScreenCount)
#define _xServerVendor      XMP(ServerVendor)
#define _xProtocolVersion   XMP(ProtocolVersion)
#define _xProtocolRevision  XMP(ProtocolRevision)
#define _xVendorRelease     XMP(VendorRelease)
#define _xDisplayString     XMP(DisplayString)
#define _xDefaultDepth      XMP(DefaultDepth)
#define _xDefaultColormap   XMP(DefaultColormap)
#define _xBitmapUnit        XMP(BitmapUnit)
#define _xBitmapBitOrder    XMP(BitmapBitOrder)
#define _xBitmapPad         XMP(BitmapPad)
#define _xImageByteOrder    XMP(ImageByteOrder)

#define _xBitmapUnit        XMP(BitmapUnit)


#define _xScreenOfDisplay         XMP(ScreenOfDisplay)
#define _xDefaultScreenOfDisplay  XMP(DefaultScreenOfDisplay)
#define _xDisplayOfScreen         XMP(DisplayOfScreen)
#define _xRootWindowOfScreen      XMP(RootWindowOfScreen)
#define _xBlackPixelOfScreen      XMP(BlackPixelOfScreen)
#define _xWhitePixelOfScreen      XMP(WhitePixelOfScreen)
#define _xDefaultColormapOfScreen XMP(DefaultColormapOfScreen)
#define _xDefaultDepthOfScreen    XMP(DefaultDepthOfScreen)
#define _xDefaultGCOfScreen       XMP(DefaultGCOfScreen)
#define _xDefaultVisualOfScreen   XMP(DefaultVisualOfScreen)
#define _xWidthOfScreen           XMP(WidthOfScreen)
#define _xHeightOfScreen          XMP(HeightOfScreen)
#define _xWidthMMOfScreen         XMP(WidthMMOfScreen)
#define _xHeightMMOfScreen        XMP(HeightMMOfScreen)
#define _xPlanesOfScreen          XMP(PlanesOfScreen)
#define _xCellsOfScreen           XMP(CellsOfScreen)
#define _xMinCmapsOfScreen        XMP(MinCmapsOfScreen)
#define _xMaxCmapsOfScreen        XMP(MaxCmapsOfScreen)
#define _xDoesSaveUnders          XMP(DoesSaveUnders)
#define _xDoesBackingStore        XMP(DoesBackingStore)
#define _xEventMaskOfScreen       XMP(EventMaskOfScreen)


// }}}
// {{{ ints 

// wraps unsigned long
#define Pixel_color_val  Unsigned_long_val
#define Val_pixel_color  Val_long

// type 'Time' defined in X.h (unsigned long / CARD32)
// use int64 instead of int32 because the max value of unsigned is twice
// (ocaml int32 is signed)
#define Val_time  caml_copy_int64
#define Time_val(t)  ((Time)Int64_val(t))

// same value than Int64.max_int (used to check Time overflow)
// (not used anymore, while CARD32 can not overflow a signed int64)
#define MAX_INT64  9223372036854775807L

// }}}
// {{{ Atom 

#define Val_Atom(v) ((value)(v))
#define Atom_val(v) ((Atom)(v))
#define Atom_val_addr(v) ((Atom *)(&v))

// }}}
// {{{ KeyCode 

/* There's no real uniformity about the type keycode,
   - in structures it is most often (int) and sometimes (unsigned int)
   - in functions sometimes there is the NeedWidePrototypes switch
     which switches between (unsigned int) and (unsigned char)
   - in functions sometimes it is just (int) as parameter 
     or (int *) if it's a returned value

   However as it handles a code for each key of the keyboard, and that generally
   keyboards have about 105 keys, it seems that it does not really matter.
*/

#define Val_KeyCode Val_long
#define KeyCode_val Long_val

// }}}
// {{{ caml_copy_string_array_n() 

// The list provided to caml_copy_string_array() needs to be NULL terminated
static value caml_copy_string_array_n(char **strl, int n)
{
    CAMLlocal1(ret);
    char const **param;
    int i;
    param = malloc((n+1) * sizeof(char *));
    for (i=0; i<n; i++) {
        param[i] = strl[i];
    }
    param[n] = NULL;  // here the point
    ret = caml_copy_string_array(param);
    free(param);
    return ret;
}
// }}}


// TODO: XGetErrorText()
#if 0
int ErrorHandler( Display *dpy, XErrorEvent *event )
{   
    char buffer[BUFSIZ];
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    /*
    event->request_code;
    event->minor_code;
    event->resourceid;
    event->serial;
    */
    printf("ERROR: %s\n", buffer);
    return 0;
} 

CAMLprim value
ml_XSetErrorHandler( value unit )
{
    XSetErrorHandler( ErrorHandler );
    return Val_unit;
}
#endif


int ErrorHandler_closure( Display *dpy, XErrorEvent *event )
{
    CAMLlocal1( ml_event );
    static value * closure_f = NULL;
    if (closure_f == NULL) {
        closure_f = caml_named_value("Error Handler Callback");
    }
    copy_XEvent( event, ml_event );
    caml_callback2( *closure_f, Val_Display(dpy), ml_event );
    return 0;
}
CAMLprim value
ml_XSetErrorHandler( value unit ) {
    XSetErrorHandler(ErrorHandler_closure);
    return Val_unit;
}




CAMLprim value
ml_XlibSpecificationRelease( value unit )
{
    return Val_int(XlibSpecificationRelease);
}

CAMLprim value
ml_XOpenDisplay( value display_name )
{
    Display *dpy;
    dpy = XOpenDisplay( String_val(display_name) );
    if (dpy == NULL) {
        caml_failwith("Cannot open display");
    }
    return Val_Display(dpy);
}

CAMLprim value
ml_XCloseDisplay( value dpy )
{
    XCloseDisplay( Display_val(dpy) );
    display_record_closed(dpy);
    return Val_unit;
}

CAMLprim value
ml_XFlush( value dpy )
{
    GET_STATUS  XFlush( Display_val(dpy) );
    CHECK_STATUS(XFlush, 1);
    return Val_unit;
}

CAMLprim value
ml_XBell( value dpy, value percent )
{
    //GET_STATUS
    XBell(
        Display_val(dpy),
        Int_val(percent)
    );
    //CHECK_STATUS(XBell,1);
    return Val_unit;
}

CAMLprim value
ml_LastKnownRequestProcessed( value dpy )
{
    return Val_long(LastKnownRequestProcessed(dpy));
}

static const int close_mode_table[] = {
    DestroyAll,
    RetainPermanent,
    RetainTemporary,
};
#define Close_mode_val(i) (close_mode_table[Long_val(i)])

CAMLprim value
ml_XSetCloseDownMode( value dpy, value close_mode )
{
    //GET_STATUS
    XSetCloseDownMode(
        Display_val(dpy),
        Close_mode_val(close_mode)
    );
    //CHECK_STATUS(XSetCloseDownMode,1);
    return Val_unit;
}

CAMLprim value
ml_XSync( value dpy, value discard )
{
    //GET_STATUS
    XSync(
        Display_val(dpy),
        Bool_val(discard)
    );
    //CHECK_STATUS(XSync,1);
    return Val_unit;
}


CAMLprim value
ml_XGrabServer( value dpy )
{
    //GET_STATUS
    XGrabServer( 
        Display_val(dpy)
    );
    //CHECK_STATUS(XGrabServer,1);
    return Val_unit;
}

static inline value Val_focus_state(int state)
{
    switch (state)
    {
        case RevertToParent:      return Val_int(0);
        case RevertToPointerRoot: return Val_int(1);
        case RevertToNone:        return Val_int(2);
        default:                  caml_failwith("unhandled focus state");
    }
}

CAMLprim value
ml_XGetInputFocus( value dpy )
{
    CAMLlocal1(pair);
    Window w;
    int revert_to;
    XGetInputFocus( Display_val(dpy), &w, &revert_to );
    pair = caml_alloc(2, 0);
    Store_field(pair, 0, (w == None) ? Val_none : Val_some(Val_Window(w)));
    Store_field(pair, 1, Val_focus_state(revert_to));
    return pair;
}

CAMLprim value
ml_XUngrabServer( value dpy )
{
    //GET_STATUS
    XUngrabServer( 
        Display_val(dpy)
    );
    //CHECK_STATUS(XUngrabServer,1);
    return Val_unit;
}

CAMLprim value
ml_XUngrabPointer( value dpy, value time )
{
    //GET_STATUS
    XUngrabPointer(
        Display_val(dpy),
        Time_val(time)
    );
    //CHECK_STATUS(XUngrabPointer,1);
    return Val_unit;
}

CAMLprim value
ml_XUngrabKeyboard( value dpy, value time )
{
    //GET_STATUS
    XUngrabKeyboard(
        Display_val(dpy),
        Time_val(time)
    );
    //CHECK_STATUS(XUngrabKeyboard,1);
    return Val_unit;
}

CAMLprim value
ml_XConnectionNumber( value dpy )
{
    return Val_int( XConnectionNumber( Display_val(dpy) ));
}

CAMLprim value
ml_XDefaultScreen( value dpy )
{
    int screen_number = _xDefaultScreen( Display_val(dpy) );
    return Val_screenNB(screen_number);
}

CAMLprim value
ml_XScreenCount( value dpy )
{
    return Val_int( _xScreenCount( Display_val(dpy) ));
}

CAMLprim value
ml_XDefaultRootWindow( value dpy )
{
    return Val_Window( _xDefaultRootWindow( Display_val(dpy) ));
}

CAMLprim value
ml_XDefaultVisual( value dpy, value screen_number )
{
    Visual * vis = _xDefaultVisual(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_Visual(vis);
}

CAMLprim value ml_Visual_visualid( value visual ) {
    return Val_VisualID( Visual_val(visual)->visualid );
}

CAMLprim value ml_Visual_map_entries( value visual ) {
    return Val_int( Visual_val(visual)->map_entries );
}

CAMLprim value ml_Visual_bits_per_rgb( value visual ) {
    return Val_int( Visual_val(visual)->bits_per_rgb );
}

CAMLprim value ml_Visual_red_mask( value visual ) {
    return Val_long( Visual_val(visual)->red_mask );
}

CAMLprim value ml_Visual_green_mask( value visual ) {
    return Val_long( Visual_val(visual)->green_mask );
}

CAMLprim value ml_Visual_blue_mask( value visual ) {
    return Val_long( Visual_val(visual)->blue_mask );
}


CAMLprim value
ml_XDefaultDepth( value dpy, value screen_number )
{
    int depth = _xDefaultDepth(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_int(depth);
}

CAMLprim value
ml_XListDepths( value dpy, value screen_number )
{
    CAMLparam2(dpy, screen_number);
    CAMLlocal1(ml_depths);

    int i, count;
    int *depths = XListDepths(
        Display_val(dpy),
        ScreenNB_val(screen_number),
        &count
    );

    ml_depths = caml_alloc(count, 0);
    for (i=0; i<count; ++i) {
        Store_field( ml_depths, i, Val_int(depths[i]) );
    }
    XFree(depths);

    CAMLreturn(ml_depths);
}

CAMLprim value
ml_XDisplayPlanes( value dpy, value screen_number )
{
    int depth = _xDisplayPlanes(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_int(depth);
}

CAMLprim value
ml_XDefaultColormap( value dpy, value screen_number )
{
    Colormap colormap = _xDefaultColormap(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_Colormap(colormap);
}

CAMLprim value
ml_XDisplayCells( value dpy, value screen_number )
{
    int cells = _xDisplayCells(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_int(cells);
}

CAMLprim value
ml_XBitmapUnit( value dpy )
{
    return Val_int( _xBitmapUnit( Display_val(dpy) ));
}

CAMLprim value
ml_XBitmapPad( value dpy )
{
    return Val_int( _xBitmapPad( Display_val(dpy) ));
}

CAMLprim value
ml_XProtocolVersion( value dpy )
{
    return Val_int( _xProtocolVersion( Display_val(dpy) ));
}

CAMLprim value
ml_XProtocolRevision( value dpy )
{
    return Val_int( _xProtocolRevision( Display_val(dpy) ));
}

CAMLprim value
ml_XVendorRelease( value dpy )
{
    return Val_int( _xVendorRelease( Display_val(dpy) ));
}

CAMLprim value
ml_XServerVendor( value dpy )
{
    char * vendor = _xServerVendor( Display_val(dpy) );
    return caml_copy_string(vendor);
}

CAMLprim value
ml_XBlackPixel( value dpy, value screen_number )
{
    unsigned long color = _xBlackPixel(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_pixel_color(color);
}

CAMLprim value
ml_XWhitePixel( value dpy, value screen_number )
{
    unsigned long color = _xWhitePixel(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_pixel_color(color);
}

CAMLprim value
ml_XDisplayWidth( value dpy, value screen_number )
{
    int width = _xDisplayWidth(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_int(width);
}

CAMLprim value
ml_XDisplayHeight( value dpy, value screen_number )
{
    int height = _xDisplayHeight(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_int(height);
}

CAMLprim value
ml_XRootWindow( value dpy, value screen_number )
{
    Window win = _xRootWindow(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_Window(win);
}

CAMLprim value
ml_XDefaultGC( value dpy, value screen_number )
{
    GC gc = _xDefaultGC(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_GC(gc,dpy);
}

/* {{{ XColor */

CAMLprim value
ml_alloc_XColor( value unit )
{
    CAMLparam0();
    CAMLlocal1(x_color);
    alloc_XColor(x_color);
    memset(XColor_val(x_color), 0, sizeof(XColor));
    CAMLreturn(x_color);
}

CAMLprim value
ml_XAllocNamedColor( value dpy, value colormap, value color_name )
{
    CAMLparam3(dpy, colormap, color_name);
    CAMLlocal3(xcolor_pair, screen_def, exact_def);

    alloc_XColor(screen_def);
    alloc_XColor(exact_def);

    //GET_STATUS
    XAllocNamedColor(
        Display_val(dpy),
        Colormap_val(colormap),
        String_val(color_name),
        XColor_val(screen_def),
        XColor_val(exact_def)
    );
    //CHECK_STATUS(XAllocNamedColor,1);

    xcolor_pair = caml_alloc(2, 0);
    Store_field( xcolor_pair, 0, screen_def );
    Store_field( xcolor_pair, 1, exact_def );
    CAMLreturn(xcolor_pair);
}

CAMLprim value
ml_XColor_set_red( value x_color, value v )
{
    XColor * xcolor = XColor_val(x_color);
    xcolor->red = Long_val(v);
    return Val_unit;
}

CAMLprim value
ml_XColor_set_green( value x_color, value v )
{
    XColor * xcolor = XColor_val(x_color);
    xcolor->green = Long_val(v);
    return Val_unit;
}

CAMLprim value
ml_XColor_set_blue( value x_color, value v )
{
    XColor * xcolor = XColor_val(x_color);
    xcolor->blue = Long_val(v);
    return Val_unit;
}

CAMLprim value
ml_XColor_set_rgb( value x_color, value r, value g, value b )
{
    XColor * xcolor = XColor_val(x_color);
    xcolor->red = Long_val(r);
    xcolor->green = Long_val(g);
    xcolor->blue = Long_val(b);
    return Val_unit;
}


static const char color_flags_table[] = {
    DoRed,
    DoGreen,
    DoBlue,
};

CAMLprim value
ml_XColor_set_flags( value x_color, value mask_list )
{
    XColor * xcolor = XColor_val(x_color);

    while ( mask_list != Val_emptylist )
    {
        value head = Field(mask_list, 0);
        xcolor->flags |= color_flags_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return Val_unit;
}

CAMLprim value
ml_XAllocColor( value dpy, value colormap, value x_color )
{
    XColor * xcolor = XColor_val(x_color);
    XAllocColor( Display_val(dpy), Colormap_val(colormap), xcolor );
    return Val_unit;
}

CAMLprim value
ml_XAllocColorCells(
        value dpy,
        value colormap,
        value contig,
        value nplanes,
        value npixels )
{
    CAMLparam5(dpy, colormap, contig, nplanes, npixels);
    CAMLlocal3(ret, pixels_arr, plnmsk_arr);
    unsigned long *pixels = NULL;
    unsigned long *plane_masks = NULL;
    long i;

    pixels = malloc(UInt_val(npixels) * sizeof(unsigned long));
    if (pixels == NULL) {
      caml_failwith("xAllocColorCells: out of memory");
    }

    plane_masks = malloc(UInt_val(nplanes) * sizeof(unsigned long));
    if (plane_masks == NULL) {
      free(pixels);
      caml_failwith("xAllocColorCells: out of memory");
    }

    Status status = XAllocColorCells(
        Display_val(dpy),
        Colormap_val(colormap),
        Bool_val(contig),
        plane_masks,
        UInt_val(nplanes),
        pixels,
        UInt_val(npixels)
    );
    if (!status) {
      free(pixels);
      free(plane_masks);
      caml_failwith("xAllocColorCells: "
                    "can't alloc enough colors in the current color map");
    }

    pixels_arr = caml_alloc(UInt_val(npixels), 0);
    for (i=0; i < UInt_val(npixels); ++i)
    {
        Store_field( pixels_arr, i, Val_ulong(pixels[i]) );
    }
    free(pixels);

    plnmsk_arr = caml_alloc(UInt_val(nplanes), 0);
    for (i=0; i < UInt_val(nplanes); ++i)
    {
        Store_field( plnmsk_arr, i, Val_ulong(plane_masks[i]) );
    }
    free(plane_masks);

    ret = caml_alloc(2, 0);
    Store_field(ret, 0, pixels_arr );
    Store_field(ret, 1, plnmsk_arr );
    CAMLreturn(ret);
}

CAMLprim value
ml_XAllocColorCellsPixels(
        value dpy,
        value colormap,
        value contig,
        value npixels )
{
    CAMLparam4(dpy, colormap, contig, npixels);
    CAMLlocal1(pixels_arr);
    unsigned long *pixels = NULL;
    long i;
    pixels = malloc(UInt_val(npixels) * sizeof(unsigned long));
    if (pixels == NULL) caml_failwith("xAllocColorCells: out of memory");

    Status status = XAllocColorCells(
        Display_val(dpy),
        Colormap_val(colormap),
        Bool_val(contig),
        NULL, 0,
        pixels,
        UInt_val(npixels)
    );
    if (!status) {
      free(pixels);
      caml_failwith("xAllocColorCells: "
                    "can't alloc enough colors in the current color map");
    }

    pixels_arr = caml_alloc(UInt_val(npixels), 0);
    for (i=0; i < UInt_val(npixels); ++i)
    {
        Store_field( pixels_arr, i, Val_ulong(pixels[i]) );
    }
    free(pixels);

    CAMLreturn(pixels_arr);
}

CAMLprim value
ml_XColor_pixel( value x_color )
{
    XColor * xcolor = XColor_val(x_color);
    return Val_pixel_color(xcolor->pixel);
}

CAMLprim value
ml_XColor_set_pixel( value x_color, value pixel_color )
{
    XColor * xcolor = XColor_val(x_color);
    xcolor->pixel = Pixel_color_val(pixel_color);
    return Val_unit;
}

CAMLprim value
ml_XQueryColor( value dpy, value colormap, value x_color )
{
    XQueryColor(
        Display_val(dpy),
        Colormap_val(colormap),
        XColor_val(x_color)
    );
    return Val_unit;
}

CAMLprim value
ml_XColor_get_red( value x_color )
{
    XColor * xcolor = XColor_val(x_color);
    return Val_long(xcolor->red);
}

CAMLprim value
ml_XColor_get_green( value x_color )
{
    XColor * xcolor = XColor_val(x_color);
    return Val_long(xcolor->green);
}

CAMLprim value
ml_XColor_get_blue( value x_color )
{
    XColor * xcolor = XColor_val(x_color);
    return Val_long(xcolor->blue);
}

CAMLprim value
ml_XColor_get_rgb( value x_color )
{
    CAMLparam1(x_color);
    CAMLlocal1(rgb);
    XColor * xcolor = XColor_val(x_color);
    rgb = caml_alloc(3, 0);
    Store_field( rgb, 0, Val_long(xcolor->red) );
    Store_field( rgb, 1, Val_long(xcolor->green) );
    Store_field( rgb, 2, Val_long(xcolor->blue) );
    CAMLreturn(rgb);
}

/* }}} */

CAMLprim value
ml_XCreateSimpleWindow( value dpy, value parent, value x, value y,
                        value width, value height, value border_width,
                        value border, value background)
{
    Window win = XCreateSimpleWindow(
        Display_val(dpy),
        Window_val(parent),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        UInt_val(border_width),
        Pixel_color_val(border),
        Pixel_color_val(background) );
    return Val_Window(win);
}
CAMLprim value
ml_XCreateSimpleWindow_bytecode( value * argv, int argn )
{
    return ml_XCreateSimpleWindow( argv[0], argv[1], argv[2], argv[3],
                                   argv[4], argv[5], argv[6], argv[7], argv[8] );
}

CAMLprim value
ml_XDestroyWindow( value dpy, value win )
{
    GET_STATUS  XDestroyWindow(
        Display_val(dpy),
        Window_val(win) );
    CHECK_STATUS(XDestroyWindow, 1);
    return Val_unit;
}

CAMLprim value
caml_get_xid(value xid)
{
    return Val_XID(Long_val(xid));
}

CAMLprim value
caml_get_xid_of_window(value win)
{
    return Val_long(Window_val(win));
}

CAMLprim value
ml_alloc_XVisualInfo( value unit )
{
    CAMLparam0();
    CAMLlocal1(visInfo);
    alloc_XVisualInfo(visInfo);
    memset(XVisualInfo_val(visInfo), 0, sizeof(XVisualInfo));
    CAMLreturn(visInfo);
}

static const long vinfo_mask_table[] = {
    VisualNoMask,
    VisualIDMask,
    VisualScreenMask,
    VisualDepthMask,
    VisualClassMask,
    VisualRedMaskMask,
    VisualGreenMaskMask,
    VisualBlueMaskMask,
    VisualColormapSizeMask,
    VisualBitsPerRGBMask,
    VisualAllMask,
};

static inline long
vinfo_mask_val( value mask_list )
{
    long c_mask = 0; 
    while ( mask_list != Val_emptylist )
    {
        value head = Field(mask_list, 0);
        c_mask |= vinfo_mask_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return c_mask;
}

CAMLprim value
ml_XGetVisualInfo( value dpy, value vinfo_mask, value vinfo_template )
{
    CAMLparam3(dpy, vinfo_mask, vinfo_template);
    CAMLlocal2(via, visual_info);
    int i, nitems;
    XVisualInfo *visInfo = XGetVisualInfo(
        Display_val(dpy),
        vinfo_mask_val(vinfo_mask),
        XVisualInfo_val(vinfo_template),
        &nitems
    );
    if (!visInfo) caml_failwith("xGetVisualInfo: can't get visual");
    via = caml_alloc(nitems, 0);
    for (i=0; i<nitems; i++) {
        alloc_XVisualInfo(visual_info);
        memcpy(XVisualInfo_val(visual_info), &(visInfo[i]), sizeof(XVisualInfo));
        //XFree(visInfo[i]);
        Store_field(via, i, visual_info);
    }
    XFree(visInfo);
    CAMLreturn(via);
}


static const int color_class_table[] = {
    StaticGray,
    GrayScale,
    StaticColor,
    PseudoColor,
    TrueColor,
    DirectColor,
};
#define Color_class_val(v) (color_class_table[Long_val(v)])

#define XVisualInfo_set_field(Conv_val, field) \
CAMLprim value ml_XVisualInfo_set_##field( value visinfo, value v ) { \
    XVisualInfo *vi = XVisualInfo_val(visinfo); \
    vi->field = Conv_val(v); \
    return Val_unit; \
}

XVisualInfo_set_field( Visual_val,      visual )
XVisualInfo_set_field( VisualID_val,    visualid )
XVisualInfo_set_field( ScreenNB_val,    screen )
XVisualInfo_set_field( Long_val,        depth )
XVisualInfo_set_field( Color_class_val, class )
XVisualInfo_set_field( ULong_val,       red_mask )
XVisualInfo_set_field( ULong_val,       green_mask )
XVisualInfo_set_field( ULong_val,       blue_mask )
XVisualInfo_set_field( Long_val,        colormap_size )
XVisualInfo_set_field( Long_val,        bits_per_rgb )


CAMLprim value
ml_XMatchVisualInfo( value dpy, value screen, value depth, value color_class )
{
    CAMLparam4(dpy, screen, depth, color_class);
    CAMLlocal1(visual_info);
    alloc_XVisualInfo(visual_info);
    Status st = XMatchVisualInfo(
        Display_val(dpy),
        ScreenNB_val(screen),
        Int_val(depth),
        Color_class_val(color_class),
        XVisualInfo_val(visual_info)
    );
    if (st == False) caml_failwith("xMatchVisualInfo: no visual found");
    CAMLreturn(visual_info);
}

CAMLprim value
ml_XVisualInfo_contents( value visual_info )
{
    CAMLparam1(visual_info);
    CAMLlocal1(dat);

    XVisualInfo * vi = XVisualInfo_val(visual_info);

    dat = caml_alloc(9, 0);
    Store_field( dat, 0, Val_Visual(vi->visual) );
    Store_field( dat, 1, Val_VisualID(vi->visualid) );
    Store_field( dat, 2, Val_screenNB(vi->screen) );
    Store_field( dat, 3, Val_int(vi->depth) );
    Store_field( dat, 4, Val_long(vi->red_mask) );
    Store_field( dat, 5, Val_long(vi->green_mask) );
    Store_field( dat, 6, Val_long(vi->blue_mask) );
    Store_field( dat, 7, Val_int(vi->colormap_size) );
    Store_field( dat, 8, Val_int(vi->bits_per_rgb) );
    CAMLreturn(dat);
}

CAMLprim value
ml_XFree_XVisualInfo( value visual_info )
{
    XVisualInfo * vi = XVisualInfo_val(visual_info);
    if (vi == NULL) {
        caml_invalid_argument("xFree_xVisualInfo: xVisualInfo NULL pointer");
    } else {
        XFree( vi );
        vi = NULL;
    }
    return Val_unit;
}

CAMLprim value
ml_XCreateColormap( value dpy, value win, value visual, value alloc )
{
    Colormap colormap = XCreateColormap(
        Display_val(dpy),
        Window_val(win),
        Visual_val(visual),
        ( Int_val(alloc) ? AllocAll : AllocNone)
    );
    return Val_Colormap(colormap);
}

CAMLprim value
ml_XFreeColormap( value dpy, value colormap )
{
    GET_STATUS  XFreeColormap(
        Display_val(dpy),
        Colormap_val(colormap) );
    CHECK_STATUS(XFreeColormap,1);
    return Val_unit;
}

CAMLprim value
ml_XCopyColormapAndFree( value dpy, value colormap )
{
    Colormap new_colormap = XCopyColormapAndFree(
        Display_val(dpy),
        Colormap_val(colormap) );
    /*
    if ((new_colormap=XCopyColormapAndFree(Display_val(dpy),
                                           Colormap_val(colormap))) == BadAlloc)
        caml_failwith("Can't Create new colormap");
    */
    return Val_Colormap(new_colormap);
}

CAMLprim value
ml_XSetWindowColormap( value dpy, value win, value colormap )
{
    GET_STATUS  XSetWindowColormap(
        Display_val(dpy),
        Window_val(win),
        Colormap_val(colormap) );
    CHECK_STATUS(XSetWindowColormap,1);
    return Val_unit;
}

CAMLprim value
_ml_XSetWindowAttributes_alloc( value unit )
{
    CAMLparam0();
    CAMLlocal2(ret, wattr);
    alloc_XSetWindowAttributes(wattr);
    ret = caml_alloc(2, 0);
    Store_field(ret, 0, wattr );
    Store_field(ret, 1, (value) 0 );
    CAMLreturn(ret);
}

CAMLprim value
ml_XSetWindowAttributes_alloc( value unit )
{
    CAMLparam0();
    CAMLlocal1(wattr);
    alloc_XSetWindowAttributes(wattr);
    CAMLreturn(wattr);
}


#define WATTR_SET(field_c_type, attr_field, field_mask, Conv_val, ml_type) \
\
CAMLprim value \
ml_xSetWindowAttributes_set_##attr_field( value ml_wattr, value _##attr_field ) \
{ \
    XSetWindowAttributes * wattr; \
    wattr = XSetWindowAttributes_val(ml_wattr); \
    wattr->attr_field = Conv_val(_##attr_field); \
    return Val_unit; \
}

/* setting the fields of the struct XSetWindowAttributes and the associated mask */

WATTR_SET( Pixmap,        background_pixmap,     CWBackPixmap,       Pixmap_val,    pixmap          )
WATTR_SET( unsigned long, background_pixel,      CWBackPixel,        Pixel_color_val, uint          )
WATTR_SET( Pixmap,        border_pixmap,         CWBorderPixmap,     Pixmap_val,    pixmap          )
WATTR_SET( unsigned long, border_pixel,          CWBorderPixel,      Pixel_color_val, uint          )
WATTR_SET( int,           bit_gravity,           CWBitGravity,       Int_val,       int             )
WATTR_SET( int,           win_gravity,           CWWinGravity,       Int_val,       int             )
WATTR_SET( int,           backing_store,         CWBackingStore,     Int_val,       int             )
WATTR_SET( unsigned long, backing_planes,        CWBackingPlanes,    ULong_val,     uint            )//XXX
WATTR_SET( unsigned long, backing_pixel,         CWBackingPixel,     ULong_val,     uint            )//pixel_color?
WATTR_SET( Bool,          save_under,            CWSaveUnder,        Bool_val,      bool            )
WATTR_SET( long,          event_mask,            CWEventMask,        Eventmask_val, event_mask_list )
WATTR_SET( long,          do_not_propagate_mask, CWDontPropagate,    Long_val,      int             )
WATTR_SET( Bool,          override_redirect,     CWOverrideRedirect, Bool_val,      bool            )
WATTR_SET( Colormap,      colormap,              CWColormap,         Colormap_val,  colormap        )
WATTR_SET( Cursor,        cursor,                CWCursor,           Cursor_val,    cursor          )

static const unsigned int window_class_table[] = {
    CopyFromParent,
    InputOutput,
    InputOnly,
};


static const unsigned long winattr_valuemask_table[] = {
    CWBackPixmap,
    CWBackPixel,
    CWBorderPixmap,
    CWBorderPixel,
    CWBitGravity,
    CWWinGravity,
    CWBackingStore,
    CWBackingPlanes,
    CWBackingPixel,
    CWOverrideRedirect,
    CWSaveUnder,
    CWEventMask,
    CWDontPropagate,
    CWColormap,
    CWCursor,
};

static inline unsigned long
winattr_valuemask_val( value mask_list )
{
    unsigned long c_mask = 0; 
    while ( mask_list != Val_emptylist )
    {
        value head = Field(mask_list, 0);
        c_mask |= winattr_valuemask_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return c_mask;
}


CAMLprim value
ml_XGetWindowAttributes( value dpy, value win )
{
    CAMLparam2(dpy, win);
    CAMLlocal1(wattr);
    alloc_XWindowAttributes(wattr);
    //GET_STATUS
    XGetWindowAttributes(
        Display_val(dpy),
        Window_val(win),
        XWindowAttributes_val(wattr)
    );
    //CHECK_STATUS(XGetWindowAttributes,1);
    CAMLreturn(wattr);
}


#define quote(s) #s

#define WATTR_GET( Val_conv, field_name, ml_type ) \
\
CAMLprim value \
ml_XWindowAttributes_##field_name( value wattr ) { \
    return Val_conv( XWindowAttributes_val(wattr)->field_name ); \
}
#define WATTR_GML( Val_conv, field_name, ml_type ) \
external xWindowAttributes_##field_name: xWindowAttributes -> ml_type = quote(ml_XWindowAttributes_##field_name)

WATTR_GET( Val_int,      x,             int )
WATTR_GET( Val_int,      y,             int )
WATTR_GET( Val_int,      width,         int )
WATTR_GET( Val_int,      height,        int )
WATTR_GET( Val_int,      depth,         int )
WATTR_GET( Val_XScreen,  screen,        xScreen )
WATTR_GET( Val_int,      border_width,  int )
WATTR_GET( Val_Colormap, colormap,      colormap )
WATTR_GET( Val_bool,     map_installed, bool )

CAMLprim value
ml_XWindowAttributes_all( value dpy, value win )
{
    CAMLparam2(dpy, win);
    CAMLlocal1(wattrs);
    XWindowAttributes c_wattr;
    //GET_STATUS
    XGetWindowAttributes(
        Display_val(dpy),
        Window_val(win),
        &c_wattr
    );
    //CHECK_STATUS(XGetWindowAttributes,1);
    wattrs = caml_alloc(5, 0);
    Store_field( wattrs, 0, Val_int( c_wattr.x ) );
    Store_field( wattrs, 1, Val_int( c_wattr.y ) );
    Store_field( wattrs, 2, Val_int( c_wattr.width ) );
    Store_field( wattrs, 3, Val_int( c_wattr.height ) );
    Store_field( wattrs, 4, Val_int( c_wattr.depth ) );
    CAMLreturn(wattrs);
}

#if 0
typedef struct {
    int x, y;                   /* location of window */
    int width, height;          /* width and height of window */
    int border_width;           /* border width of window */
    int depth;                  /* depth of window */
    Visual *visual;             /* the associated visual structure */
    Window root;                /* root of screen containing window */
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;                /* C++ InputOutput, InputOnly*/
#else
    int class;                  /* InputOutput, InputOnly*/
#endif
    int bit_gravity;            /* one of bit gravity values */
    int win_gravity;            /* one of the window gravity values */
    int backing_store;          /* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to be used when restoring planes */
    Bool save_under;            /* boolean, should bits under be saved? */
    Colormap colormap;          /* color map to be associated with window */
    Bool map_installed;         /* boolean, is color map currently installed*/
    int map_state;              /* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;       /* set of events all people have interest in*/
    long your_event_mask;       /* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    Bool override_redirect;     /* boolean value for override-redirect */
    Screen *screen;             /* back pointer to correct screen */
} XWindowAttributes;
#endif


CAMLprim value
ml_XCreateWindow(
        value dpy, value parent,
        value x, value y,
        value width, value height,
        value border_width,
        value depth, value class, value visual,
        value valuemask, value attributes )
{
    Window win = XCreateWindow(
        Display_val(dpy),
        Window_val(parent),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        UInt_val(border_width),
        Int_val(depth),
        window_class_table[Long_val(class)],
        Visual_val(visual),
        winattr_valuemask_val(valuemask),
        XSetWindowAttributes_val(attributes)
    );
    if (!win) caml_failwith("XCreateWindow");
    return Val_Window(win);
}
CAMLprim value
ml_XCreateWindow_bytecode( value * argv, int argn )
{
    return ml_XCreateWindow( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5],
                             argv[6], argv[7], argv[8], argv[9], argv[10], argv[11] );
}

CAMLprim value
ml_XResizeWindow( value dpy, value win, value width, value height )
{
    GET_STATUS  XResizeWindow(
        Display_val(dpy),
        Window_val(win),
        UInt_val(width),
        UInt_val(height)
    );
    CHECK_STATUS(XResizeWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XMoveWindow( value dpy, value win, value x, value y )
{
    GET_STATUS  XMoveWindow(
        Display_val(dpy),
        Window_val(win),
        Int_val(x),
        Int_val(y)
    );
    CHECK_STATUS(XMoveWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XMoveResizeWindow( value dpy, value win, value x, value y, value width, value height )
{
    GET_STATUS  XMoveResizeWindow(
        Display_val(dpy),
        Window_val(win),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height)
    );
    CHECK_STATUS(XMoveResizeWindow,1);
    return Val_unit;
}
CAMLprim value
ml_XMoveResizeWindow_bytecode( value * argv, int argn )
{
    return ml_XMoveResizeWindow( argv[0], argv[1], argv[2],
                                 argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_XLowerWindow( value dpy, value win )
{
    GET_STATUS  XLowerWindow(
        Display_val(dpy),
        Window_val(win) );
    CHECK_STATUS(XLowerWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XRaiseWindow( value dpy, value win )
{
    GET_STATUS  XRaiseWindow(
        Display_val(dpy),
        Window_val(win) );
    CHECK_STATUS(XRaiseWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XStoreName( value dpy, value win, value name )
{
    GET_STATUS XStoreName(  
        Display_val(dpy),
        Window_val(win),
        String_val(name) );
    CHECK_STATUS(XStoreName, 1);
    return Val_unit;
}

CAMLprim value
ml_XFetchName( value dpy, value win )
{
    CAMLlocal1( ml_window_name );
    char * window_name = NULL;
    //GET_STATUS
    XFetchName(
        Display_val(dpy),
        Window_val(win),
        &window_name
    );
    //CHECK_STATUS(XFetchName,1);
    if (window_name != NULL) {
        ml_window_name = caml_copy_string(window_name);
        XFree(window_name);
    } else {
        caml_failwith("xFetchName");
    }
    return ml_window_name;
}

CAMLprim value
ml_XSelectInput( value dpy, value win, value ml_event_mask )
{
    long event_mask = Eventmask_val( ml_event_mask );
    GET_STATUS  XSelectInput(
        Display_val(dpy),
        Window_val(win),
        event_mask );
    CHECK_STATUS(XSelectInput, 1);
    return Val_unit;
}

CAMLprim value
ml_XMapWindow( value dpy, value win )
{
    GET_STATUS  XMapWindow(
        Display_val(dpy),
        Window_val(win) );
    CHECK_STATUS(XMapWindow, 1);
    return Val_unit;
}

CAMLprim value
ml_XMapSubwindows( value dpy, value win )
{
    GET_STATUS  XMapSubwindows(
        Display_val(dpy),
        Window_val(win) );
    CHECK_STATUS(XMapSubwindows, 1);
    return Val_unit;
}

CAMLprim value
ml_XMapRaised( value dpy, value win )
{
    //GET_STATUS
    XMapRaised(
        Display_val(dpy),
        Window_val(win) );
    //CHECK_STATUS(XMapRaised, 1);
    return Val_unit;
}

CAMLprim value
ml_XUnmapWindow( value dpy, value win )
{
    //GET_STATUS
    XUnmapWindow(
        Display_val(dpy),
        Window_val(win) );
    //CHECK_STATUS(XUnmapWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XReparentWindow( value dpy, value win, value parent, value x, value y )
{
    GET_STATUS  XReparentWindow(
        Display_val(dpy),
        Window_val(win),
        Window_val(parent),
        Int_val(x),
        Int_val(y)
    );
    CHECK_STATUS(XReparentWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XChangeSaveSet( value dpy, value win, value change_mode )
{
    //GET_STATUS
    XChangeSaveSet(
        Display_val(dpy),
        Window_val(win),
        (Int_val(change_mode) ? SetModeDelete : SetModeInsert)
    );
    //CHECK_STATUS(XChangeSaveSet,1);
    return Val_unit;
}

CAMLprim value
ml_XAddToSaveSet( value dpy, value win )
{
    //GET_STATUS
    XAddToSaveSet(
        Display_val(dpy),
        Window_val(win)
    );
    //CHECK_STATUS(XAddToSaveSet,1);
    return Val_unit;
}

CAMLprim value
ml_XRemoveFromSaveSet( value dpy, value win )
{
    //GET_STATUS
    XRemoveFromSaveSet(
        Display_val(dpy),
        Window_val(win)
    );
    //CHECK_STATUS(XRemoveFromSaveSet,1);
    return Val_unit;
}


CAMLprim value
ml_XQueryTree( value dpy, value win )
{
    CAMLparam2( dpy, win );
    CAMLlocal2( ret, children_arr );

    Window root_win, parent_win, *children;
    unsigned int nchildren, i;

    children = NULL;
    Status status = XQueryTree(
        Display_val(dpy),
        Window_val(win),
        &root_win,
        &parent_win,
        &children,
        &nchildren
    );
    if (status != 1) {
        if (children != NULL) XFree(children);
        caml_failwith("xQueryTree");
    }

    children_arr = caml_alloc(nchildren, 0);
    for (i=0; i < nchildren; i++) {
        Store_field( children_arr, i, Val_Window(children[i]) );
    }
    XFree(children);

    ret = caml_alloc(3, 0);
    Store_field( ret, 0, Val_Window(root_win) );
    Store_field( ret, 1, Val_Window(parent_win) );
    Store_field( ret, 2, children_arr );

    CAMLreturn( ret );
}

CAMLprim value
ml_XRestackWindows( value dpy, value ml_wins )
{
    int nwindows, i;
    Window* windows;
    nwindows = Wosize_val(ml_wins);
    windows = malloc(nwindows * sizeof(Window*));
    for (i=0; i < nwindows; i++) {
       windows[i] = Window_val(Field(ml_wins, i));
    }
    //GET_STATUS
    XRestackWindows(
        Display_val(dpy),
        windows,
        nwindows
    );
    free(windows);
    //CHECK_STATUS(XRestackWindows,1);
    return Val_unit;
}


static const int circulateSubwinsDir_table[] = {
    RaiseLowest,
    LowerHighest
};
#define CirculateSubwinsDir_val(i) (circulateSubwinsDir_table[Long_val(i)])

CAMLprim value
ml_XCirculateSubwindows( value dpy, value win, value dir )
{
    //GET_STATUS
    XCirculateSubwindows(
        Display_val(dpy),
        Window_val(win),
        CirculateSubwinsDir_val(dir)
    );
    //CHECK_STATUS(XCirculateSubwindows,1);
    return Val_unit;
}

CAMLprim value
ml_XCirculateSubwindowsDown( value dpy, value win )
{
    //GET_STATUS
    XCirculateSubwindowsDown(
        Display_val(dpy),
        Window_val(win)
    );
    //CHECK_STATUS(XCirculateSubwindowsDown,1);
    return Val_unit;
}

CAMLprim value
ml_XCirculateSubwindowsUp( value dpy, value win )
{
    //GET_STATUS
    XCirculateSubwindowsUp(
        Display_val(dpy),
        Window_val(win)
    );
    //CHECK_STATUS(XCirculateSubwindowsUp,1);
    return Val_unit;
}


CAMLprim value
ml_XGetWindowProperty_string(
        value dpy,
        value win,
        value property,
        value long_offset,
        value long_length,
        value delete,
        value req_type
        )
{
    CAMLparam5(dpy, win, property, long_offset, long_length);
    CAMLxparam2(delete, req_type);
    CAMLlocal1(ret);

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    /*unsigned*/ char* prop;

    (void) XGetWindowProperty(
        Display_val(dpy),
        Window_val(win),
        Atom_val(property),
        Long_val(long_offset),
        Long_val(long_length),
        Bool_val(delete),
        AnyPropertyType,  // Atom req_type,  TODO
        &actual_type,
        &actual_format,
        &nitems,
        &bytes_after,
        (unsigned char**)&prop
    );
    ret = caml_alloc(5, 0);
    Store_field(ret, 0, Val_Atom(actual_type) );
    Store_field(ret, 1, Val_int(actual_format) );
    Store_field(ret, 2, Val_long(nitems) );
    Store_field(ret, 3, Val_long(bytes_after) );
    Store_field(ret, 4, caml_copy_string(prop) );
    XFree(prop);
    CAMLreturn(ret);
}
CAMLprim value
ml_XGetWindowProperty_string_bytecode( value * argv, int argn )
{
    return ml_XGetWindowProperty_string( argv[0], argv[1], argv[2],
                                         argv[3], argv[4], argv[5], argv[6] );
}

CAMLprim value
ml_XGetWindowProperty_window(
        value dpy,
        value win,
        value property,
        value long_offset,
        value long_length,
        value delete,
        value req_type
        )
{
    CAMLparam5(dpy, win, property, long_offset, long_length);
    CAMLxparam2(delete, req_type);
    CAMLlocal1(ret);

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    Window *prop;

    (void) XGetWindowProperty(
        Display_val(dpy),
        Window_val(win),
        Atom_val(property),
        Long_val(long_offset),
        Long_val(long_length),
        Bool_val(delete),
        AnyPropertyType,  // Atom req_type,   TODO
        &actual_type,
        &actual_format,
        &nitems,
        &bytes_after,
        (unsigned char**)&prop
    );
    ret = caml_alloc(5, 0);
    Store_field(ret, 0, Val_Atom(actual_type) );
    Store_field(ret, 1, Val_int(actual_format) );
    Store_field(ret, 2, Val_long(nitems) );
    Store_field(ret, 3, Val_long(bytes_after) );
    Store_field(ret, 4, Val_Window(prop[0]) );
    XFree(prop);
    CAMLreturn(ret);
}
CAMLprim value
ml_XGetWindowProperty_window_bytecode( value * argv, int argn )
{
    return ml_XGetWindowProperty_window( argv[0], argv[1], argv[2],
                                         argv[3], argv[4], argv[5], argv[6] );
}

CAMLprim value
ml_hasWindowProperty(
        value dpy,
        value win,
        value property
        )
{
    Atom actual_type;
    Atom expected_type = Atom_val(property);
    int actual_format;
    unsigned long nitems, bytes_after;
    Window *prop;

    int result = XGetWindowProperty(
        Display_val(dpy),
        Window_val(win),
        expected_type,
        Long_val(0), // offset
        Long_val(0), // length
        0,           // delete
        expected_type,
        &actual_type,
        &actual_format,
        &nitems,
        &bytes_after,
        (unsigned char**)&prop
    );
    if (result != Success)
        return Val_false;

    XFree(prop);

    if (actual_type != property)
        return Val_false;

    return Val_true;
}


/* Managing Installed Colormaps */

CAMLprim value
ml_XInstallColormap( value dpy, value colormap )
{
    //GET_STATUS
    XInstallColormap(
        Display_val(dpy),
        Colormap_val(colormap)
    );
    //CHECK_STATUS(XInstallColormap,1);
    return Val_unit;
}

CAMLprim value
ml_XUninstallColormap( value dpy, value colormap )
{
    //GET_STATUS
    XUninstallColormap(
        Display_val(dpy),
        Colormap_val(colormap)
    );
    //CHECK_STATUS(XUninstallColormap,1);
    return Val_unit;
}

CAMLprim value
ml_XListInstalledColormaps( value dpy, value win )
{
    CAMLparam2(dpy, win);
    CAMLlocal1(ret);
    int i, num;
    Colormap *colormaps =
        XListInstalledColormaps(
            Display_val(dpy),
            Window_val(win),
            &num
        );
    ret = caml_alloc(num, 0);
    for (i=0; i<num; ++i) {
        Store_field(ret, i, Val_Colormap(colormaps[i]) );
    }
    XFree(colormaps);
    CAMLreturn(ret);
}


CAMLprim value
ml_XKillClient( value dpy, value resource )
{
    //GET_STATUS
    XKillClient(
        Display_val(dpy),
        (XID) resource
    );
    //CHECK_STATUS(XKillClient,1);
    return Val_unit;
}

/* Threads */

CAMLprim value
ml_XInitThreads( value unit )
{
    GET_STATUS XInitThreads();
    CHECK_STATUS(XInitThreads,1);
    return Val_unit;
}

CAMLprim value
ml_XLockDisplay( value dpy )
{
    XLockDisplay( Display_val(dpy) );
    return Val_unit;
}

CAMLprim value
ml_XUnlockDisplay( value dpy )
{
    XUnlockDisplay( Display_val(dpy) );
    return Val_unit;
}



CAMLprim value
ml_XSetWMProtocols( value dpy, value win, value protocols, value count )
{
    Status status = XSetWMProtocols(
        Display_val(dpy),
        Window_val(win),
        Atom_val_addr(protocols),
        Int_val(count) );
    CHECK_STATUS(XSetWMProtocols, 1);
    return Val_unit;
}

CAMLprim value
ml_XInternAtom( value dpy, value atom_name, value only_if_exists )
{
    Atom a = XInternAtom(
        Display_val(dpy),
        String_val(atom_name),
        Bool_val(only_if_exists) );
    if (a == None)
        caml_raise_not_found();
    // XInternAtom() can generate BadAlloc and BadValue errors.
    return Val_Atom(a);
}

CAMLprim value
ml_XInternAtoms( value dpy, value ml_names, value only_if_exists )
{
    CAMLparam3(dpy, ml_names, only_if_exists);
    CAMLlocal1(ret);
    int count, i;
    char** names;
    Atom* atoms_return;
    count = Wosize_val(ml_names);
    atoms_return = malloc(count * sizeof(Atom));
    names = malloc(count * sizeof(char *));
    for (i=0; i<count; ++i) {
        names[i] = String_val(Field(ml_names,i));
    }
    Status st = XInternAtoms(
        Display_val(dpy),
        names,
        count,
        Bool_val(only_if_exists),
        atoms_return
    );
    if (st == 0)
        caml_failwith("xInternAtoms: atoms were not returned for all of the names");

    ret = caml_alloc(count, 0);
    for (i=0; i<count; ++i) {
        Store_field( ret, i, Val_Atom(atoms_return[i]) );
    }
    free(atoms_return);
    free(names);
    CAMLreturn(ret);
}

CAMLprim value
ml_XGetAtomName( value dpy, value atom )
{
    CAMLparam2(dpy, atom);
    CAMLlocal1(ml_atom_name);
    char * atom_name = XGetAtomName(
        Display_val(dpy),
        Atom_val(atom)
    );
    if (atom_name == NULL)
        caml_failwith("xGetAtomName");
    ml_atom_name = caml_copy_string(atom_name);
    XFree((void *)atom_name);
    CAMLreturn(ml_atom_name);
}


/* XSizeHints, from <X11/Xutil.h> */

CAMLprim value
ml_alloc_XSizeHints( value unit )
{
    CAMLparam0();
    CAMLlocal1(size_hints);
    alloc_XSizeHints(size_hints);
    memset(XSizeHints_val(size_hints), 0, sizeof(XSizeHints));
    CAMLreturn(size_hints);
}

CAMLprim value
ml_XSizeHints_set_USPosition( value size_hints, value _x, value _y )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= USPosition;
    sh->x = Int_val(_x);
    sh->y = Int_val(_y);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PPosition( value size_hints, value _x, value _y )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PPosition;
    sh->x = Int_val(_x);
    sh->y = Int_val(_y);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_USSize( value size_hints, value _width, value _height )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= USSize;
    sh->width = Int_val(_width);
    sh->height = Int_val(_height);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PSize( value size_hints, value _width, value _height )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PSize;
    sh->width = Int_val(_width);
    sh->height = Int_val(_height);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PMinSize( value size_hints, value _min_width, value _min_height )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PMinSize;
    sh->min_width = Int_val(_min_width);
    sh->min_height = Int_val(_min_height);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PMaxSize( value size_hints, value _max_width, value _max_height )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PMaxSize;
    sh->max_width = Int_val(_max_width);
    sh->max_height = Int_val(_max_height);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PResizeInc( value size_hints, value _width_inc, value _height_inc )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PResizeInc;
    sh->width_inc = Int_val(_width_inc);
    sh->height_inc = Int_val(_height_inc);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PBaseSize( value size_hints, value _base_width, value _base_height )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PBaseSize;
    sh->base_width = Int_val(_base_width);
    sh->base_height = Int_val(_base_height);
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PAspect( value size_hints, value min_aspect, value max_aspect )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PAspect;
    sh->min_aspect.x = Int_val(Field(min_aspect,0));
    sh->min_aspect.y = Int_val(Field(min_aspect,1));
    sh->max_aspect.x = Int_val(Field(max_aspect,0));
    sh->max_aspect.y = Int_val(Field(max_aspect,1));
    return Val_unit;
}

CAMLprim value
ml_XSizeHints_set_PWinGravity( value size_hints, value _win_gravity )
{
    XSizeHints *sh;
    sh = XSizeHints_val(size_hints);
    sh->flags |= PWinGravity;
    sh->win_gravity = Int_val(_win_gravity);
    return Val_unit;
}

CAMLprim value
ml_XSetNormalHints( value dpy, value win, value size_hints )
{
    XSizeHints* hints;
    hints = XSizeHints_val(size_hints);
    GET_STATUS  XSetNormalHints(
        Display_val(dpy),
        Window_val(win),
        hints );
    CHECK_STATUS(XSetNormalHints,1);
    return Val_unit;
}


CAMLprim value
ml_XSetStandardProperties(
        value dpy, value win,
        value window_name,
        value icon_name,
        value ml_icon_pixmap,
        value ml_argv,
        value hints )
{
    int argc = Wosize_val(ml_argv);
    char ** argv = malloc((argc+1) * sizeof(char*));
    int i;
    for (i=0; i<argc; i++) {
        value ml_arg = Field(ml_argv,i);
        int len = caml_string_length(ml_arg);
        char *arg = String_val(ml_arg);
        argv[i] = malloc((len+1) * sizeof(char));
        strncpy(argv[i], arg, len);
        argv[i][len] = '\0';
    }
    argv[argc] = NULL;

    Pixmap icon_pixmap;
    if (ml_icon_pixmap == Val_int(0)) icon_pixmap = None;  // None
    else icon_pixmap = Pixmap_val( Field(ml_icon_pixmap,0) );  // Some v

    GET_STATUS  XSetStandardProperties(
        Display_val(dpy),
        Window_val(win),
        String_val(window_name),
        String_val(icon_name),
        icon_pixmap,
        argv, argc,
        XSizeHints_val(hints) );

    for (i=0; i<argc; ++i) {
        free(argv[i]);
    }
    free(argv);

    CHECK_STATUS(XSetStandardProperties,1);
    return Val_unit;
}
CAMLprim value
ml_XSetStandardProperties_bytecode( value * argv, int argn )
{
    return ml_XSetStandardProperties( argv[0], argv[1], argv[2],
                                      argv[3], argv[4], argv[5], argv[6] );
}


CAMLprim value
ml_alloc_XGCValues( value unit )
{
    CAMLparam0();
    CAMLlocal1(gcv);
    alloc_XGCValues(gcv);
    memset(XGCValues_val(gcv), 0, sizeof(XGCValues));
    CAMLreturn(gcv);
}

static const unsigned long gc_valuemask_table[] = {
    GCFunction,
    GCPlaneMask,
    GCForeground,
    GCBackground,
    GCLineWidth,
    GCLineStyle,
    GCCapStyle,
    GCJoinStyle,
    GCFillStyle,
    GCFillRule,
    GCTile,
    GCStipple,
    GCTileStipXOrigin,
    GCTileStipYOrigin,
    GCFont,
    GCSubwindowMode,
    GCGraphicsExposures,
    GCClipXOrigin,
    GCClipYOrigin,
    GCClipMask,
    GCDashOffset,
    GCDashList,
    GCArcMode,
};

static inline unsigned long
gc_valuemask_val( value mask_list )
{
    unsigned long c_mask = 0; 
    while ( mask_list != Val_emptylist )
    {
        value head = Field(mask_list, 0);
        c_mask |= gc_valuemask_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return c_mask;
}


CAMLprim value
ml_XCreateGC( value dpy, value d, value valuemask, value gc_values )
{
    CAMLparam4( dpy, d, valuemask, gc_values );
    GC gc = XCreateGC(
        Display_val(dpy),
        Drawable_val(d),
        gc_valuemask_val(valuemask),
        XGCValues_val(gc_values)
    );
    if (!gc)
        caml_failwith("xCreateGC: out of memory");
    CAMLreturn( Val_GC_final(gc,dpy) );
}

CAMLprim value
ml_XChangeGC( value dpy, value gc, value valuemask, value gc_values )
{
    //GET_STATUS
    XChangeGC(
        Display_val(dpy),
        GC_val(gc),
        gc_valuemask_val(valuemask),
        XGCValues_val(gc_values)
    );
    //CHECK_STATUS(XChangeGC,1);
    return Val_unit;
}

CAMLprim value
ml_XGetGCValues( value dpy, value gc, value valuemask )
{
    CAMLparam3(dpy, gc, valuemask);
    CAMLlocal1(gcv);
    alloc_XGCValues(gcv);
    GET_STATUS  XGetGCValues(
        Display_val(dpy),
        GC_val(gc),
        gc_valuemask_val(valuemask),
        XGCValues_val(gcv)
    );
    CHECK_STATUS(XGetGCValues,True);
    CAMLreturn(gcv);
}


static const int logical_operation_function_table[] = {
    GXclear,
    GXand,
    GXandReverse,
    GXcopy,
    GXandInverted,
    GXnoop,
    GXxor,
    GXor,
    GXnor,
    GXequiv,
    GXinvert,
    GXorReverse,
    GXcopyInverted,
    GXorInverted,
    GXnand,
    GXset
};
#define Function_val(i) (logical_operation_function_table[Long_val(i)])
static inline value Val_function(int function) {
    switch (function) {
        case GXclear:        return Val_int(0);
        case GXand:          return Val_int(1);
        case GXandReverse:   return Val_int(2);
        case GXcopy:         return Val_int(3);
        case GXandInverted:  return Val_int(4);
        case GXnoop:         return Val_int(5);
        case GXxor:          return Val_int(6);
        case GXor:           return Val_int(7);
        case GXnor:          return Val_int(8);
        case GXequiv:        return Val_int(9);
        case GXinvert:       return Val_int(10);
        case GXorReverse:    return Val_int(11);
        case GXcopyInverted: return Val_int(12);
        case GXorInverted:   return Val_int(13);
        case GXnand:         return Val_int(14);
        case GXset:          return Val_int(15);
    }
    return Val_int(0);
}

static const int line_style_table[] = {
    LineSolid,
    LineOnOffDash,
    LineDoubleDash
};
#define Line_style_val(i) (line_style_table[Long_val(i)])
static inline value Val_line_style(int line_style) {
    switch (line_style) {
        case LineSolid:      return Val_int(0);
        case LineOnOffDash:  return Val_int(1);
        case LineDoubleDash: return Val_int(2);
    }
    return Val_int(0);
}

static const int cap_style_table[] = {
    CapNotLast,
    CapButt,
    CapRound,
    CapProjecting
};
#define Cap_style_val(i) (cap_style_table[Long_val(i)])
static inline value Val_cap_style(int cap_style) {
    switch (cap_style) {
        case CapNotLast:    return Val_int(0);
        case CapButt:       return Val_int(1);
        case CapRound:      return Val_int(2);
        case CapProjecting: return Val_int(3);
    }
    return Val_int(0);
}

static const int join_style_table[] = {
    JoinMiter,
    JoinRound,
    JoinBevel
};
#define Join_style_val(i) (join_style_table[Long_val(i)])
static inline value Val_join_style(int join_style) {
    switch (join_style) {
        case JoinMiter: return Val_int(0);
        case JoinRound: return Val_int(1);
        case JoinBevel: return Val_int(2);
    }
    return Val_int(0);
}

static const int fill_style_table[] = {
    FillSolid,
    FillTiled,
    FillStippled,
    FillOpaqueStippled
};
#define Fill_style_val(i) (fill_style_table[Long_val(i)])
static inline value Val_fill_style(int fill_style) {
    switch (fill_style) {
        case FillSolid:          return Val_int(0);
        case FillTiled:          return Val_int(1);
        case FillStippled:       return Val_int(2);
        case FillOpaqueStippled: return Val_int(3);
    }
    return Val_int(0);
}

static const int fill_rule_table[] = {
    EvenOddRule,
    WindingRule
};
#define Fill_rule_val(i) (fill_rule_table[Long_val(i)])
static inline value Val_fill_rule(int fill_rule) {
    switch (fill_rule) {
        case EvenOddRule: return Val_int(0);
        case WindingRule: return Val_int(1);
    }
    return Val_int(0);
}

static const int arc_mode_table[] = {
    ArcChord,
    ArcPieSlice
};
#define Arc_mode_val(i) (arc_mode_table[Long_val(i)])
static inline value Val_arc_mode(int arc_mode) {
    switch (arc_mode) {
        case ArcChord:    return Val_int(0);
        case ArcPieSlice: return Val_int(1);
    }
    return Val_int(0);
}

static const int subwindow_mode_table[] = {
    ClipByChildren,
    IncludeInferiors
};
#define Subwindow_mode_val(i) (subwindow_mode_table[Long_val(i)])
static inline value Val_subwindow_mode(int subwindow_mode) {
    switch (subwindow_mode) {
        case ClipByChildren:   return Val_int(0);
        case IncludeInferiors: return Val_int(1);
    }
    return Val_int(0);
}



#define GCVAL_SET(field_c_type, field_name, Conv_val, Val_conv, ml_type, mask) \
\
CAMLprim value \
ml_XGCValues_set_##field_name( value ml_gcv, value v ) \
{ \
    XGCValues * gcv; \
    gcv = XGCValues_val(ml_gcv); \
    gcv->field_name = Conv_val(v); \
    return Val_unit; \
}

#define GCVAL_GET(field_c_type, field_name, Conv_val, Val_conv, ml_type, mask) \
\
CAMLprim value \
ml_XGCValues_get_##field_name( value ml_gcv, value v ) \
{ \
    XGCValues * gcv; \
    gcv = XGCValues_val(ml_gcv); \
    return Val_conv( gcv->field_name ); \
}

#define GCVAL_SML(field_c_type, field_name, Conv_val, Val_conv, ml_type, mask) \
external xGCValues_set_##field_name: gcv:xGCValues -> ml_type -> unit = quote(ml_XGCValues_set_##field_name)

#define GCVAL_GML(field_c_type, field_name, Conv_val, Val_conv, ml_type, mask) \
external xGCValues_get_##field_name: gcv:xGCValues -> ml_type = quote(ml_XGCValues_get_##field_name)

GCVAL_SET( unsigned long, foreground,         Pixel_color_val,    Val_pixel_color,    pixel_color,    GCForeground        )
GCVAL_SET( unsigned long, background,         Pixel_color_val,    Val_pixel_color,    pixel_color,    GCBackground        )
GCVAL_SET( Bool,          graphics_exposures, Bool_val,           Val_bool,           bool,           GCGraphicsExposures )
GCVAL_SET( Pixmap,        tile,               Pixmap_val,         Val_Pixmap,         pixmap,         GCTile              )
GCVAL_SET( int,           clip_x_origin,      Int_val,            Val_int,            int,            GCClipXOrigin       )
GCVAL_SET( int,           clip_y_origin,      Int_val,            Val_int,            int,            GCClipYOrigin       )
GCVAL_SET( int,           ts_x_origin,        Int_val,            Val_int,            int,            GCTileStipXOrigin   )
GCVAL_SET( int,           ts_y_origin,        Int_val,            Val_int,            int,            GCTileStipYOrigin   )
GCVAL_SET( int,           line_style,         Line_style_val,     Val_line_style,     line_style,     GCLineStyle         )
GCVAL_SET( int,           cap_style,          Cap_style_val,      Val_cap_style,      cap_style,      GCCapStyle          )
GCVAL_SET( int,           join_style,         Join_style_val,     Val_join_style,     join_style,     GCJoinStyle         )
GCVAL_SET( int,           fill_style,         Fill_style_val,     Val_fill_style,     fill_style,     GCFillStyle         )
GCVAL_SET( int,           fill_rule,          Fill_rule_val,      Val_fill_rule,      fill_rule,      GCFillRule          )
GCVAL_SET( int,           function,           Function_val,       Val_function,       logop_func,     GCFunction          )
GCVAL_SET( int,           line_width,         Int_val,            Val_int,            int,            GCLineWidth         )
GCVAL_SET( int,           arc_mode,           Arc_mode_val,       Val_arc_mode,       arc_mode,       GCArcMode           )
GCVAL_SET( Font,          font,               Font_val,           Val_Font,           font,           GCFont              )
GCVAL_SET( int,           subwindow_mode,     Subwindow_mode_val, Val_subwindow_mode, subwindow_mode, GCSubwindowMode     )


GCVAL_GET( unsigned long, foreground,         Pixel_color_val,    Val_pixel_color,    pixel_color,    GCForeground        )
GCVAL_GET( unsigned long, background,         Pixel_color_val,    Val_pixel_color,    pixel_color,    GCBackground        )
GCVAL_GET( Bool,          graphics_exposures, Bool_val,           Val_bool,           bool,           GCGraphicsExposures )
GCVAL_GET( Pixmap,        tile,               Pixmap_val,         Val_Pixmap,         pixmap,         GCTile              )
GCVAL_GET( int,           clip_x_origin,      Int_val,            Val_int,            int,            GCClipXOrigin       )
GCVAL_GET( int,           clip_y_origin,      Int_val,            Val_int,            int,            GCClipYOrigin       )
GCVAL_GET( int,           ts_x_origin,        Int_val,            Val_int,            int,            GCTileStipXOrigin   )
GCVAL_GET( int,           ts_y_origin,        Int_val,            Val_int,            int,            GCTileStipYOrigin   )
GCVAL_GET( int,           line_style,         Line_style_val,     Val_line_style,     line_style,     GCLineStyle         )
GCVAL_GET( int,           cap_style,          Cap_style_val,      Val_cap_style,      cap_style,      GCCapStyle          )
GCVAL_GET( int,           join_style,         Join_style_val,     Val_join_style,     join_style,     GCJoinStyle         )
GCVAL_GET( int,           fill_style,         Fill_style_val,     Val_fill_style,     fill_style,     GCFillStyle         )
GCVAL_GET( int,           fill_rule,          Fill_rule_val,      Val_fill_rule,      fill_rule,      GCFillRule          )
GCVAL_GET( int,           function,           Function_val,       Val_function,       logop_func,     GCFunction          )
GCVAL_GET( int,           line_width,         Int_val,            Val_int,            int,            GCLineWidth         )
GCVAL_GET( int,           arc_mode,           Arc_mode_val,       Val_arc_mode,       arc_mode,       GCArcMode           )
GCVAL_GET( Font,          font,               Font_val,           Val_Font,           font,           GCFont              )
GCVAL_GET( int,           subwindow_mode,     Subwindow_mode_val, Val_subwindow_mode, subwindow_mode, GCSubwindowMode     )

/*
                       | GCPlaneMask
                       | GCStipple
                       | GCClipMask
                       | GCDashOffset
                       | GCDashList
*/

#if 0
      XGCValues gcv;

      gcv.plane_mask = 0x1;
      gcv.stipple = bitmap;
      gcv.clip_mask = bitmap;
      gcv.dash_offset = 1;
      gcv.dashes = 0xc2;

      GC gc = XCreateGC(dpy, w,
                          GCFunction | GCPlaneMask | GCForeground | GCBackground | GCLineWidth | GCLineStyle
                        | GCCapStyle | GCJoinStyle | GCFillStyle | GCFillRule | GCTile | GCStipple
                        | GCTileStipXOrigin | GCTileStipYOrigin | GCFont | GCSubwindowMode | GCGraphicsExposures
                        | GCClipXOrigin | GCClipYOrigin | GCClipMask | GCDashOffset | GCDashList | GCArcMode,
                        &gcv);

        unsigned long plane_mask;/* plane mask */
        Pixmap stipple;         /* stipple 1 plane pixmap for stipping */
        int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
        Pixmap clip_mask;       /* bitmap clipping; other calls for rects */
        int dash_offset;        /* patterned/dashed line information */
        char dashes;
#endif


CAMLprim value
ml_XSetForeground( value dpy, value gc, value foreground )
{
    GET_STATUS  XSetForeground(
        Display_val(dpy),
        GC_val(gc),
        Pixel_color_val(foreground)
    );
    CHECK_STATUS(XSetForeground,1);
    return Val_unit;
}

CAMLprim value
ml_XSetBackground( value dpy, value gc, value background )
{
    GET_STATUS  XSetBackground(
        Display_val(dpy),
        GC_val(gc),
        Pixel_color_val(background)
    );
    CHECK_STATUS(XSetBackground,1);
    return Val_unit;
}

CAMLprim value
ml_XSetLineAttributes( value dpy, value gc, value line_width, value line_style,
                       value cap_style, value join_style )
{
    GET_STATUS  XSetLineAttributes(
        Display_val(dpy),
        GC_val(gc),
        UInt_val(line_width),
        Line_style_val(line_style),
        Cap_style_val(cap_style),
        Join_style_val(join_style)
    );
    CHECK_STATUS(XSetLineAttributes,1);
    return Val_unit;
}
CAMLprim value
ml_XSetLineAttributes_bytecode( value * argv, int argn )
{
    return ml_XSetLineAttributes( argv[0], argv[1], argv[2],
                                  argv[3], argv[4], argv[5] );
}

CAMLprim value ml_XSetFillStyle( value dpy, value gc, value fill_style )
{
    XSetFillStyle(
        Display_val(dpy),
        GC_val(gc),
        Fill_style_val(fill_style)
    ); 
    return Val_unit;
}

CAMLprim value
ml_XClearWindow( value dpy, value win )
{
    GET_STATUS  XClearWindow(
        Display_val(dpy),
        Window_val(win)
    );
    CHECK_STATUS(XClearWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XClearArea( value dpy, value win, value x, value y, value width, value height, value exposures )
{
    //GET_STATUS
    XClearArea(
        Display_val(dpy),
        Window_val(win),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        Bool_val(exposures)
    );
    //CHECK_STATUS(XClearArea,1);
    return Val_unit;
}
CAMLprim value
ml_XClearArea_bytecode( value * argv, int argn )
{
    return ml_XClearArea( argv[0], argv[1], argv[2],
                          argv[3], argv[4], argv[5], argv[6] );
}

/*
TODO: Max number of elements:

XDrawLines() points:  XMaxRequestSize(dpy) - 3
XFillPolygon() points:  XMaxRequestSize(dpy) - 4
XDrawSegments() segments:  (XMaxRequestSize(dpy) - 3) / 2
XDrawRectangles() rectangles:  (XMaxRequestSize(dpy) - 3) / 2
XFillRectangles() rectangles:  (XMaxRequestSize(dpy) - 3) / 2
XDrawArcs() or XFillArcs() arcs:  (XMaxRequestSize(dpy) - 3) / 3
*/



CAMLprim value
ml_XDrawArc( value dpy, value d, value gc, value x, value y,
             value width, value height, value angle1, value angle2 )
{
    GET_STATUS  XDrawArc(    
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        Int_val(angle1),
        Int_val(angle2)
    ); 
    CHECK_STATUS(XDrawArc, 1);
    return Val_unit;
}
CAMLprim value
ml_XDrawArc_bytecode( value * argv, int argn )
{
    return ml_XDrawArc( argv[0], argv[1], argv[2], argv[3],
                        argv[4], argv[5], argv[6], argv[7], argv[8] );
}

CAMLprim value
ml_XDrawArcs( value dpy, value d, value gc, value ml_arcs )
{
    int i, narcs = Wosize_val(ml_arcs);
    XArc * arcs = malloc(narcs * sizeof(XArc));
    for (i=0; i<narcs; ++i) {
        value a = Field(ml_arcs, i);
        arcs[i].x = Int_val(Field(a,0));
        arcs[i].y = Int_val(Field(a,1));
        arcs[i].width = UInt_val(Field(a,2));
        arcs[i].height = UInt_val(Field(a,3));
        arcs[i].angle1 = Int_val(Field(a,4));
        arcs[i].angle2 = Int_val(Field(a,5));
    }
    GET_STATUS  XDrawArcs(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        arcs,
        narcs
    );
    free(arcs);
    CHECK_STATUS(XDrawArcs,1);
    return Val_unit;
}


CAMLprim value
ml_XDrawImageString( value dpy, value d, value gc, value x, value y, value str )
{
    GET_STATUS  XDrawImageString(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        String_val(str),
        caml_string_length(str)
    );
    CHECK_STATUS(XDrawImageString, 0);
    return Val_unit;
}
CAMLprim value
ml_XDrawImageString_bytecode( value * argv, int argn )
{
    return ml_XDrawImageString( argv[0], argv[1], argv[2],
                                argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_alloc_XChar2b( value b2 )
{
    CAMLparam1(b2);
    CAMLlocal1(xchar2b);
    XChar2b *char16;
    alloc_XChar2b(xchar2b);
    char16 = XChar2b_val(xchar2b);
    char16->byte1 = (unsigned char) Long_val(Field(b2,0));
    char16->byte2 = (unsigned char) Long_val(Field(b2,1));
    CAMLreturn(xchar2b);
}

CAMLprim value
ml_alloc_XChar2b_string( value b2_string )
{
    CAMLparam1(b2_string);
    CAMLlocal2(ret, xchar2b_str);
    XChar2b *char16;
    long i, n;

    n = Wosize_val(b2_string);
    alloc_n_XChar2b(xchar2b_str, n);
    char16 = XChar2b_ptr_val(xchar2b_str);

    for (i=0; i < n; ++i)
    {
        value b2 = Field(b2_string, i);
        char16[i].byte1 = (unsigned char) Long_val(Field(b2,0));
        char16[i].byte2 = (unsigned char) Long_val(Field(b2,1));
    }

    ret = caml_alloc(2, 0);
    Store_field(ret, 0, xchar2b_str);
    Store_field(ret, 1, Val_long(n) );
    CAMLreturn(ret);
}

CAMLprim value
ml_XDrawImageString16( value dpy, value d, value gc, value x, value y, value xchar2b_string )
{
    GET_STATUS  XDrawImageString16(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        XChar2b_string_val(xchar2b_string),
        XChar2b_string_length(xchar2b_string)
    );
    CHECK_STATUS(XDrawImageString16,0);
    return Val_unit;
}
CAMLprim value
ml_XDrawImageString16_bytecode( value * argv, int argn )
{
    return ml_XDrawImageString16( argv[0], argv[1], argv[2],
                                  argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_XDrawLine( value dpy, value d, value gc, value x1, value y1, value x2, value y2 )
{
    GET_STATUS  XDrawLine(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x1),
        Int_val(y1),
        Int_val(x2),
        Int_val(y2)
    );
    CHECK_STATUS(XDrawLine,1);
    return Val_unit;
}
CAMLprim value
ml_XDrawLine_bytecode( value * argv, int argn )
{
    return ml_XDrawLine( argv[0], argv[1], argv[2],
                         argv[3], argv[4], argv[5], argv[6] );
}

static const int coordinate_mode_table[] = {
    CoordModeOrigin,
    CoordModePrevious,
};

CAMLprim value
ml_XDrawLines( value dpy, value d, value gc, value ml_points, value ml_mode )
{
    int mode = coordinate_mode_table[ Long_val(ml_mode) ];
    int i, npoints = Wosize_val(ml_points);
    XPoint * points = malloc(npoints * sizeof(XPoint));
    for (i=0; i<npoints; ++i)
    {
        value pnt = Field(ml_points, i);
        points[i].x = Long_val(Field(pnt, 0));
        points[i].y = Long_val(Field(pnt, 1));
    }
    GET_STATUS  XDrawLines(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        points,
        npoints,
        mode
    );
    free(points);
    CHECK_STATUS(XDrawLines,1);
    return Val_unit;
}

CAMLprim value
ml_XDrawPoint( value dpy, value d, value gc, value x, value y )
{
    GET_STATUS  XDrawPoint(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y)
    );
    CHECK_STATUS(XDrawPoint,1);
    return Val_unit;
}


CAMLprim value
ml_XDrawPoints( value dpy, value d, value gc, value ml_points, value ml_mode )
{
    int mode = coordinate_mode_table[ Long_val(ml_mode) ];
    int i, npoints = Wosize_val(ml_points);
    XPoint * points = malloc(npoints * sizeof(XPoint));
    for (i=0; i<npoints; ++i)
    {
        value pnt = Field(ml_points, i);
        points[i].x = Long_val(Field(pnt, 0));
        points[i].y = Long_val(Field(pnt, 1));
    }
    GET_STATUS  XDrawPoints(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        points,
        npoints,
        mode
    );
    CHECK_STATUS(XDrawPoints,1);
    return Val_unit;
}


CAMLprim value
ml_XDrawRectangle( value dpy, value d, value gc, value x, value y,
                   value width, value height )
{
    GET_STATUS  XDrawRectangle(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height)
    );
    CHECK_STATUS(XDrawRectangle, 1);
    return Val_unit;
}
CAMLprim value
ml_XDrawRectangle_bytecode( value * argv, int argn )
{
    return ml_XDrawRectangle( argv[0], argv[1], argv[2],
                              argv[3], argv[4], argv[5], argv[6] );
}

 
CAMLprim value
ml_XDrawRectangles( value dpy, value d, value gc, value ml_rectangles )
{
    int i, nrectangles = Wosize_val(ml_rectangles);
    XRectangle * rectangles = malloc(nrectangles * sizeof(XRectangle));
    for (i=0; i<nrectangles; ++i) {
        value rect = Field(ml_rectangles, i);
        rectangles[i].x = Int_val(Field(rect,0));
        rectangles[i].y = Int_val(Field(rect,1));
        rectangles[i].width = UInt_val(Field(rect,2));
        rectangles[i].height = UInt_val(Field(rect,3));
    }
    GET_STATUS  XDrawRectangles(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        rectangles,
        nrectangles
    );
    free(rectangles);
    CHECK_STATUS(XDrawRectangles,1);
    return Val_unit;
}


CAMLprim value
ml_XDrawSegments( value dpy, value d, value gc, value ml_segments )
{
    XSegment* segments;
    int nsegments = Wosize_val(ml_segments);
    int i;
    segments = malloc(nsegments * sizeof(XSegment));
    for (i=0; i<nsegments; ++i) {
        value seg = Field(ml_segments, i);
        segments[i].x1 = Int_val(Field(seg,0));
        segments[i].y1 = Int_val(Field(seg,1));
        segments[i].x2 = Int_val(Field(seg,2));
        segments[i].y2 = Int_val(Field(seg,3));
    }
    GET_STATUS  XDrawSegments(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        segments,
        nsegments
    );
    free(segments);
    CHECK_STATUS(XDrawSegments,1);
    return Val_unit;
}


CAMLprim value
ml_XDrawString( value dpy, value d, value gc, value x, value y, value str )
{
    GET_STATUS  XDrawString(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        String_val(str),
        caml_string_length(str)
    );
    CHECK_STATUS(XDrawString, 0);
    return Val_unit;
}
CAMLprim value
ml_XDrawString_bytecode( value * argv, int argn )
{
    return ml_XDrawString( argv[0], argv[1], argv[2],
                           argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_XDrawString16( value dpy, value d, value gc, value x, value y, value xchar2b_string )
{
    GET_STATUS  XDrawString16(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        XChar2b_string_val(xchar2b_string),
        XChar2b_string_length(xchar2b_string)
    );
    CHECK_STATUS(XDrawString16, 0);
    return Val_unit;
}
CAMLprim value
ml_XDrawString16_bytecode( value * argv, int argn )
{
    return ml_XDrawString16( argv[0], argv[1], argv[2],
                             argv[3], argv[4], argv[5] );
}

/*
int XDrawText(
    Display*     display,
    Drawable     d,
    GC           gc,
    int          x,
    int          y,
    XTextItem*   items,
    int          nitems
);

int XDrawText16(
    Display*       display,
    Drawable       d,
    GC             gc,
    int            x,
    int            y,
    XTextItem16*   items,
    int            nitems
);
*/




CAMLprim value
ml_XFillArc( value dpy, value d, value gc, value x, value y,
             value width, value height, value angle1, value angle2 )
{
    GET_STATUS  XFillArc(    
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        Int_val(angle1),
        Int_val(angle2)
    ); 
    CHECK_STATUS(XFillArc, 1);
    return Val_unit;
}
CAMLprim value
ml_XFillArc_bytecode( value * argv, int argn )
{
    return ml_XFillArc( argv[0], argv[1], argv[2], argv[3],
                        argv[4], argv[5], argv[6], argv[7], argv[8] );
}

CAMLprim value
ml_XFillArcs( value dpy, value d, value gc, value ml_arcs )
{
    int i, narcs = Wosize_val(ml_arcs);
    XArc * arcs = malloc(narcs * sizeof(XArc));
    for (i=0; i<narcs; ++i) {
        value a = Field(ml_arcs, i);
        arcs[i].x = Int_val(Field(a,0));
        arcs[i].y = Int_val(Field(a,1));
        arcs[i].width = UInt_val(Field(a,2));
        arcs[i].height = UInt_val(Field(a,3));
        arcs[i].angle1 = Int_val(Field(a,4));
        arcs[i].angle2 = Int_val(Field(a,5));
    }
    GET_STATUS  XFillArcs(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        arcs,
        narcs
    );
    free(arcs);
    CHECK_STATUS(XFillArcs,1);
    return Val_unit;
}

static const int shape_kind_table[] = {
    Complex,
    Nonconvex,
    Convex,
};

CAMLprim value
ml_XFillPolygon(
        value dpy,
        value d,
        value gc,
        value ml_points,
        value ml_shape,
        value ml_mode )
{
    int shape = shape_kind_table[ Long_val(ml_shape) ];
    int mode = coordinate_mode_table[ Long_val(ml_mode) ];
    int i, npoints = Wosize_val(ml_points);
    XPoint * points = malloc(npoints * sizeof(XPoint));
    for (i=0; i<npoints; ++i)
    {
        value pnt = Field(ml_points, i);
        points[i].x = Long_val(Field(pnt, 0));
        points[i].y = Long_val(Field(pnt, 1));
    }
    GET_STATUS  XFillPolygon(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        points,
        npoints,
        shape,
        mode
    );
    free(points);
    CHECK_STATUS(XFillPolygon,1);
    return Val_unit;
}
CAMLprim value
ml_XFillPolygon_bytecode( value * argv, int argn )
{
    return ml_XFillPolygon( argv[0], argv[1], argv[2],
                            argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_XFillRectangle( value dpy, value d, value gc, value x, value y,
                   value width, value height )
{
    GET_STATUS  XFillRectangle(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height)
    );
    CHECK_STATUS(XFillRectangle, 1);
    return Val_unit;
}
CAMLprim value
ml_XFillRectangle_bytecode( value * argv, int argn )
{
    return ml_XFillRectangle( argv[0], argv[1], argv[2],
                              argv[3], argv[4], argv[5], argv[6] );
}

CAMLprim value
ml_XFillRectangles( value dpy, value d, value gc, value ml_rectangles )
{
    int i, nrectangles = Wosize_val(ml_rectangles);
    XRectangle * rectangles = malloc(nrectangles * sizeof(XRectangle));
    for (i=0; i<nrectangles; ++i) {
        value rect = Field(ml_rectangles, i);
        rectangles[i].x = Int_val(Field(rect,0));
        rectangles[i].y = Int_val(Field(rect,1));
        rectangles[i].width = UInt_val(Field(rect,2));
        rectangles[i].height = UInt_val(Field(rect,3));
    }
    GET_STATUS  XFillRectangles(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        rectangles,
        nrectangles
    );
    free(rectangles);
    CHECK_STATUS(XFillRectangles,1);
    return Val_unit;
}

CAMLprim value
ml_XCopyArea( value dpy,
        value src,
        value dest,
        value gc,
        value src_x,
        value src_y,
        value width,
        value height,
        value dest_x,
        value dest_y )
{
    GET_STATUS  XCopyArea(
        Display_val(dpy),
        Drawable_val(src),
        Drawable_val(dest),
        GC_val(gc),
        Int_val(src_x),
        Int_val(src_y),
        UInt_val(width),
        UInt_val(height),
        Int_val(dest_x),
        Int_val(dest_y)
    );
    CHECK_STATUS(XCopyArea,1);
    return Val_unit;
}
CAMLprim value
ml_XCopyArea_bytecode( value * argv, int argn )
{
    return ml_XCopyArea( argv[0], argv[1], argv[2], argv[3], argv[4],
                         argv[5], argv[6], argv[7], argv[8], argv[9] );
}

CAMLprim value
ml_XCreatePixmap( value dpy,
        value dbl,
        value width,
        value height,
        value depth )
{
    Pixmap pixmap = XCreatePixmap(
        Display_val(dpy),
        Drawable_val(dbl),
        UInt_val(width),
        UInt_val(height),
        UInt_val(depth)
    );
    return Val_Pixmap(pixmap);
}

CAMLprim value
ml_XFreePixmap( value dpy, value pixmap )
{
    GET_STATUS  XFreePixmap(
        Display_val(dpy),
        Pixmap_val(pixmap)
    );
    CHECK_STATUS(XFreePixmap,1);
    return Val_unit;
}

CAMLprim value
ml_XCreateBitmapFromData( value dpy, value dbl, value data, value width, value height )
{
    unsigned int _width  = UInt_val(width);
    unsigned int _height = UInt_val(height);
    unsigned int len = caml_string_length(data);
    if (len < (_width * _height) / 8) {
        caml_invalid_argument("xCreateBitmapFromData");
    }
    Pixmap pixmap = XCreateBitmapFromData(
        Display_val(dpy),
        Drawable_val(dbl),
        String_val(data),
        _width,
        _height
    );
    return Val_Pixmap(pixmap);
}

CAMLprim value
ml_XCreatePixmapCursor( value dpy, value source, value mask,
                        value foreground, value background, value x, value y )
{
    Cursor cur = XCreatePixmapCursor(
        Display_val(dpy),
        Pixmap_val(source),
        Pixmap_val(mask),
        XColor_val(foreground),
        XColor_val(background),
        UInt_val(x),
        UInt_val(y)
    );
    return Val_Cursor(cur);
}
CAMLprim value
ml_XCreatePixmapCursor_bytecode( value * argv, int argn )
{
    return ml_XCreatePixmapCursor( argv[0], argv[1], argv[2],
                                   argv[3], argv[4], argv[5], argv[6] );
}

CAMLprim value
ml_XQueryBestTile( value dpy, value dbl, value width, value height )
{
    CAMLparam1(dpy);
    CAMLlocal1(size);
    unsigned int width_return, height_return;
    GET_STATUS  XQueryBestTile(
        Display_val(dpy),
        Drawable_val(dbl),
        UInt_val(width),
        UInt_val(height),
        &width_return,
        &height_return
    );
    CHECK_STATUS(XQueryBestTile,1);
    size = caml_alloc(2, 0);
    Store_field( size, 0, Val_uint(width_return) );
    Store_field( size, 1, Val_uint(height_return) );
    CAMLreturn(size);
}

CAMLprim value
ml_XListPixmapFormats( value dpy )
{
    CAMLparam1(dpy);
    CAMLlocal2(arr, v);
    int i, count = 0;
    XPixmapFormatValues * pfv = NULL;
    pfv = XListPixmapFormats(
        Display_val(dpy),
        &count
    );
    if (pfv == NULL)
        caml_failwith("xListPixmapFormats: out of memory");
    arr = caml_alloc(count, 0);
    for (i=0; i<count; ++i)
    {
        v = caml_alloc(3, 0);
        Store_field(v, 0, Val_int(pfv[i].depth));
        Store_field(v, 1, Val_int(pfv[i].bits_per_pixel));
        Store_field(v, 2, Val_int(pfv[i].scanline_pad));
 
        Store_field(arr, i, v);
    }
    XFree(pfv);
    CAMLreturn(arr);
}


/* XImage */

CAMLprim value
ml_XImageByteOrder( value dpy )
{
    int order = _xImageByteOrder( Display_val(dpy) );
    switch (order) {
        case LSBFirst: return Val_int(0);
        case MSBFirst: return Val_int(1);
    }
    caml_failwith("xImageByteOrder");
    return Val_unit;
}

static const int ximage_format_table[] = {
    XYBitmap,
    XYPixmap,
    ZPixmap,
};
#define XImage_format_val(i) (ximage_format_table[Long_val(i)])

#define Val_XImage(d) ((value)(d))
#define XImage_val(v) ((XImage *)(v))

CAMLprim value
ml_XCreateImage( value dpy, value visual, value depth, value format, value offset,
                 value data, value width, value height, value bitmap_pad, value bytes_per_line )
{
    char* _data;
    _data = (char *)((Tag_val(data) == String_tag)? (String_val(data)) : (Caml_ba_data_val(data)));

    XImage *ximage = XCreateImage(
        Display_val(dpy),
        Visual_val(visual),
        Int_val(depth),
        XImage_format_val(format),
        Int_val(offset),
        _data,
        UInt_val(width),
        UInt_val(height),
        Int_val(bitmap_pad),  /* XXX */
        Int_val(bytes_per_line) /* XXX */
    );
    return Val_XImage(ximage);
}
CAMLprim value
ml_XCreateImage_bytecode( value * argv, int argn )
{
    return ml_XCreateImage( argv[0], argv[1], argv[2], argv[3], argv[4],
                            argv[5], argv[6], argv[7], argv[8], argv[9] );
}

CAMLprim value
ml_XDestroyImage( value ximage )
{
    //GET_STATUS
    XDestroyImage(
        XImage_val(ximage)
    );
    //CHECK_STATUS(XDestroyImage,1);
    return Val_unit;
}

CAMLprim value
ml_XSubImage( value ximage, value x, value y, value width, value height )
{
    XImage *sub_image = XSubImage(
        XImage_val(ximage),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height)
    );
    return Val_XImage(sub_image);
}

CAMLprim value
ml_XAllPlanes( value unit )
{
    return Val_ulong(XAllPlanes());  // TODO: maybe switch to an ocaml int32
}


CAMLprim value
ml_XGetImage( value dpy, value d, value x, value y,
              value width, value height, value plane_mask, value _format )
{
    XImage *ximage;
    int format = XImage_format_val(_format);
    if (format == XYBitmap)
      caml_invalid_argument("xGetImage: format should be XYPixmap or ZPixmap");
 
    /*
       plane_mask represents an (unsigned long) and
       OCaml ints are C (long) and XAllPlanes()
       returns all bits set to 1 so even the signed
       bit. So we can not use the macro ULong_val()
       but Long_val()
       XXX: Maybe we should use an ocaml int32 ?
    */
 
    ximage = XGetImage(
        Display_val(dpy),
        Drawable_val(d),
        Int_val(x),
        Int_val(y),
        UInt_val(width),
        UInt_val(height),
        ULong_val(plane_mask), // ULong_val(plane_mask),
        format
    );
    if(ximage == NULL) {
        caml_failwith("XGetImage: xImage could not be created");
    }
    return Val_XImage(ximage);
}
CAMLprim value
ml_XGetImage_bytecode( value * argv, int argn )
{
    return ml_XGetImage( argv[0], argv[1], argv[2], argv[3],
                         argv[4], argv[5], argv[6], argv[7] );
}


/* TODO:

XImage *XGetSubImage(
    Display*       display,
    Drawable       d,
    int            x,
    int            y,
    unsigned int   width,
    unsigned int   height,
    unsigned long  plane_mask,
    int            format,
    XImage*        dest_image,
    int            dest_x,
    int            dest_y
);
*/

CAMLprim value
ml_XImage_data_str( value ximage )
{
    CAMLparam1(ximage);
    CAMLlocal1(ml_data);
    XImage *xim = XImage_val(ximage);
    unsigned long size = /* xim->width * */ xim->height * xim->bytes_per_line;
    void *data_ptr = (void*) (&(xim->data[0]));
    ml_data = caml_alloc_string(size);
    memcpy( String_val(ml_data), data_ptr, size );
    CAMLreturn(ml_data);
}

CAMLprim value
ml_XImage_data_ba( value ximage )
{
    CAMLparam1(ximage);
    CAMLlocal1(img_ba);
    XImage *xim = XImage_val(ximage);
    unsigned long size = /* xim->width * */ xim->height * xim->bytes_per_line;
    void *data_ptr = (void*) (&(xim->data[0]));
    long dims[3];
    dims[0] = xim->width;
    dims[1] = xim->height;
    //dims[2] = xim->depth;   /* TODO: DEBUG ME! */
    dims[2] = xim->bytes_per_line / xim->width;
    img_ba = caml_ba_alloc(CAML_BA_UINT8 | CAML_BA_C_LAYOUT, 3, NULL, dims);
    memcpy( Caml_ba_data_val(img_ba), data_ptr, size );
    CAMLreturn(img_ba);
}


CAMLprim value
ml_XGetPixel( value ximage, value x, value y )
{
    unsigned long pixel = XGetPixel(
        XImage_val(ximage),
        Int_val(x),
        Int_val(y)
    );
    return Val_pixel_color(pixel);
}

CAMLprim value
ml_XPutPixel( value ximage, value x, value y, value pixel )
{
    //GET_STATUS
    XPutPixel(
        XImage_val(ximage),
        Int_val(x),
        Int_val(y),
        Pixel_color_val(pixel)
    );
    //CHECK_STATUS(XPutPixel,1);
    return Val_unit;
}

CAMLprim value
ml_XAddPixel( value ximage, value v )
{
    //GET_STATUS
    XAddPixel(
        XImage_val(ximage),
        Long_val(v)
    );
    //CHECK_STATUS(XAddPixel,0);
    return Val_unit;
}

CAMLprim value
ml_XPutImage( value dpy, value d, value gc, value ximage,
              value src_x, value src_y, value dest_x, value dest_y,
              value width, value height )
{
    //GET_STATUS
    XPutImage(
        Display_val(dpy),
        Drawable_val(d),
        GC_val(gc),
        XImage_val(ximage),
        Int_val(src_x),
        Int_val(src_y),
        Int_val(dest_x),
        Int_val(dest_y),
        UInt_val(width),
        UInt_val(height)
    );
    //CHECK_STATUS(XPutImage,0);
    return Val_unit;
}
CAMLprim value
ml_XPutImage_bytecode( value * argv, int argn )
{
    return ml_XPutImage( argv[0], argv[1], argv[2], argv[3], argv[4],
                         argv[5], argv[6], argv[7], argv[8], argv[9] );
}


/* Font */

CAMLprim value
ml_XLoadFont( value dpy, value name )
{
    Font font = XLoadFont(
        Display_val(dpy),
        String_val(name) );
    return Val_Font(font);
}


/* Setting and Retrieving the Font Search Path */

CAMLprim value
ml_XSetFontPath( value dpy, value ml_directories )
{
    char** directories;
    int ndirs, i;
    ndirs = Wosize_val(ml_directories);
    directories = malloc(ndirs * sizeof(char*));
    for (i=0; i < ndirs; i++)
    {
        directories[i] = String_val(Field(ml_directories, i));
    }
    GET_STATUS XSetFontPath(
        Display_val(dpy),
        directories,
        ndirs
    );
    free(directories);
    CHECK_STATUS(XSetFontPath,1);
    return Val_unit;
}

CAMLprim value
ml_XGetFontPath( value dpy )
{
    CAMLlocal1(ml_paths);
    int npaths;
    char **paths = XGetFontPath(
        Display_val(dpy),
        &npaths
    );
    ml_paths = caml_copy_string_array_n(paths, npaths);
    XFreeFontPath(paths);
    return ml_paths;
}

CAMLprim value
ml_XListFonts( value dpy, value pattern, value maxnames )
{
    CAMLlocal1(ml_list);
    int actual_count;
    char **list = XListFonts(
        Display_val(dpy),
        String_val(pattern),
        Int_val(maxnames),
        &actual_count
    );
    if (list == NULL) {
        caml_failwith("no matching font names");
    }
    ml_list = caml_copy_string_array_n(list, actual_count);
    XFreeFontNames(list);
    return ml_list;
}

#if 0

TODO:

char **XListFontsWithInfo(
    Display*            /* display */,
    _Xconst char*       /* pattern */,
    int                 /* maxnames */,
    int*                /* count_return */,
    XFontStruct**       /* info_return */
);

char **XListExtensions(
    Display*            /* display */,
    int*                /* nextensions_return */
);

#endif


/* XEvent */

CAMLprim value
ml_alloc_XEvent( value unit )
{
    CAMLparam0();
    CAMLlocal1(event);
    alloc_XEvent(event);
    memset(XEvent_val(event), 0, sizeof(XEvent));
    CAMLreturn(event);
}

CAMLprim value
ml_XNextEvent( value dpy, value event )
{
    GET_STATUS  XNextEvent(
        Display_val(dpy),
        XEvent_val(event) );
    CHECK_STATUS(XNextEvent, 0);
    return Val_unit;
}

CAMLprim value
ml_XPeekEvent( value dpy, value event )
{
    GET_STATUS  XPeekEvent(
        Display_val(dpy),
        XEvent_val(event) );
    CHECK_STATUS(XPeekEvent, 1);
    return Val_unit;
}

CAMLprim value
ml_XNextEvent_fun( value dpy )
{
    CAMLparam1( dpy );
    CAMLlocal1( ml_event );
    XEvent event;
    GET_STATUS  XNextEvent(
        Display_val(dpy),
        &event );
    CHECK_STATUS(XNextEvent, 0);
    copy_XEvent( event, ml_event );
    CAMLreturn( ml_event );
}

CAMLprim value
ml_XNextEvent_fun_2( value dpy ) // TODO test me
{
    CAMLparam1(dpy);
    CAMLlocal1(event);
    alloc_XEvent(event);
    GET_STATUS  XNextEvent(
        Display_val(dpy),
        XEvent_val(event) );
    CHECK_STATUS(XNextEvent, 0);
    CAMLreturn(event);
}

CAMLprim value
ml_XMaskEvent( value dpy, value event_mask_list, value event )
{
    long event_mask = Eventmask_val( event_mask_list );
    GET_STATUS  XMaskEvent(
        Display_val(dpy),
        event_mask,
        XEvent_val(event) );
    CHECK_STATUS(XMaskEvent, 0);
    return Val_unit;
}

CAMLprim value
ml_XWindowEvent( value dpy, value win, value event_mask_list )
{
    CAMLparam3( dpy, win, event_mask_list );
    CAMLlocal1( event );
    long event_mask = Eventmask_val( event_mask_list );
    XEvent event_return;
    GET_STATUS  XWindowEvent(
        Display_val(dpy),
        Window_val(win),
        event_mask,
        &event_return );
    CHECK_STATUS(XWindowEvent, 0);
    copy_XEvent(event_return, event);
    CAMLreturn( event );
}

CAMLprim value
ml_XPending( value dpy )
{
    return Val_int(XPending(
        Display_val(dpy) ));
}

static const int event_mode_table[] = {
    AsyncPointer,
    SyncPointer,
    AsyncKeyboard,
    SyncKeyboard,
    ReplayPointer,
    ReplayKeyboard,
    AsyncBoth,
    SyncBoth,
};
#define Event_mode_val(i) (event_mode_table[Long_val(i)])

CAMLprim value
ml_XAllowEvents( value dpy, value event_mode, value time )
{
    //GET_STATUS
    XAllowEvents(
        Display_val(dpy),
        Event_mode_val(event_mode),
        Time_val(time)
    );
    //CHECK_STATUS(XAllowEvents,1);
    return Val_unit;
}

CAMLprim value
ml_XPutBackEvent( value dpy, value event )
{
    GET_STATUS  XPutBackEvent(
        Display_val(dpy),
        XEvent_val(event) );
    CHECK_STATUS(XPutBackEvent, 0);
    return Val_unit;
}

static const int event_type_table[] = {
    KeyPress,
    KeyRelease,
    ButtonPress,
    ButtonRelease,
    MotionNotify,
    EnterNotify,
    LeaveNotify,
    FocusIn,
    FocusOut,
    KeymapNotify,
    Expose,
    GraphicsExpose,
    NoExpose,
    VisibilityNotify,
    CreateNotify,
    DestroyNotify,
    UnmapNotify,
    MapNotify,
    MapRequest,
    ReparentNotify,
    ConfigureNotify,
    ConfigureRequest,
    GravityNotify,
    ResizeRequest,
    CirculateNotify,
    CirculateRequest,
    PropertyNotify,
    SelectionClear,
    SelectionRequest,
    SelectionNotify,
    ColormapNotify,
    ClientMessage,
    MappingNotify,
};

CAMLprim value
ml_XCheckTypedEvent( value dpy, value event_type, value event_return )
{
    if (XCheckTypedEvent(
        Display_val(dpy),
        event_type_table[Long_val(event_type)],
        XEvent_val(event_return) ))
        return Val_true;
    else
        return Val_false;
}


static const int queued_mode_table[] = {
    QueuedAlready,
    QueuedAfterFlush,
    QueuedAfterReading
};

CAMLprim value
ml_XEventsQueued( value dpy, value mode_i )
{
    int mode = queued_mode_table[Long_val(mode_i)];
    int n = XEventsQueued(
        Display_val(dpy),
        mode );
    return Val_int(n);
}

static inline value Val_event_type(int type)
{
    switch (type)
    {
        case KeyPress:         return Val_int(0);
        case KeyRelease:       return Val_int(1);
        case ButtonPress:      return Val_int(2);
        case ButtonRelease:    return Val_int(3);
        case MotionNotify:     return Val_int(4);
        case EnterNotify:      return Val_int(5);
        case LeaveNotify:      return Val_int(6);
        case FocusIn:          return Val_int(7);
        case FocusOut:         return Val_int(8);
        case KeymapNotify:     return Val_int(9);
        case Expose:           return Val_int(10);
        case GraphicsExpose:   return Val_int(11);
        case NoExpose:         return Val_int(12);
        case VisibilityNotify: return Val_int(13);
        case CreateNotify:     return Val_int(14);
        case DestroyNotify:    return Val_int(15);
        case UnmapNotify:      return Val_int(16);
        case MapNotify:        return Val_int(17);
        case MapRequest:       return Val_int(18);
        case ReparentNotify:   return Val_int(19);
        case ConfigureNotify:  return Val_int(20);
        case ConfigureRequest: return Val_int(21);
        case GravityNotify:    return Val_int(22);
        case ResizeRequest:    return Val_int(23);
        case CirculateNotify:  return Val_int(24);
        case CirculateRequest: return Val_int(25);
        case PropertyNotify:   return Val_int(26);
        case SelectionClear:   return Val_int(27);
        case SelectionRequest: return Val_int(28);
        case SelectionNotify:  return Val_int(29);
        case ColormapNotify:   return Val_int(30);
        case ClientMessage:    return Val_int(31);
        case MappingNotify:    return Val_int(32);
        default: caml_failwith("unhandled event type");
    }
    return Val_unit;
}

CAMLprim value
ml_XEvent_type( value event )
{
    return Val_event_type(XEvent_val(event)->type);
}


/* {{{ Get XEvents datas */

CAMLprim value
ml_XAnyEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e = XEvent_val(event);
    dat = caml_alloc(5, 0);
    Store_field( dat, 0, Val_event_type(e->xany.type) );
    Store_field( dat, 1, Val_ulong(e->xany.serial) );
    Store_field( dat, 2, Val_bool(e->xany.send_event) );
    Store_field( dat, 3, Val_Display(e->xany.display) );
    Store_field( dat, 4, Val_Window(e->xany.window) );
    CAMLreturn( dat );
}

#define CHECK_EVENT_TYPE 1

// with 64 bit integers there's no need to check Time overflow anymore
#define CHECK_TIME_OVERFLOW 0

static const unsigned int logical_state_mask_table[] = {
    AnyModifier,
    Button1Mask,
    Button2Mask,
    Button3Mask,
    Button4Mask,
    Button5Mask,
    ShiftMask,
    LockMask,
    ControlMask,
    Mod1Mask,
    Mod2Mask,
    Mod3Mask,
    Mod4Mask,
    Mod5Mask
};
//define State_mask_val(i) (logical_state_mask_table[Long_val(i)])
static unsigned int State_mask_val(li)
{
    int c_mask = 0; 
    while ( li != Val_emptylist )
    {
        value head = Field(li, 0);
        c_mask |= logical_state_mask_table[Long_val(head)];
        li = Field(li, 1);
    }
    return c_mask;
}

#define Val_AnyModifier  Val_int(0)
#define Val_Button1Mask  Val_int(1)
#define Val_Button2Mask  Val_int(2)
#define Val_Button3Mask  Val_int(3)
#define Val_Button4Mask  Val_int(4)
#define Val_Button5Mask  Val_int(5)
#define Val_ShiftMask    Val_int(6)
#define Val_LockMask     Val_int(7)
#define Val_ControlMask  Val_int(8)
#define Val_Mod1Mask     Val_int(9)
#define Val_Mod2Mask     Val_int(10)
#define Val_Mod3Mask     Val_int(11)
#define Val_Mod4Mask     Val_int(12)
#define Val_Mod5Mask     Val_int(13)

static inline value Val_state_mask(unsigned int c_mask)
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

#define push_mask(mask, Val_mask) \
    if (c_mask & mask) { \
        cons = caml_alloc(2, 0); \
        Store_field(cons, 0, Val_mask); \
        Store_field(cons, 1, li); \
        li = cons; \
    }

    push_mask( AnyModifier, Val_AnyModifier )
    push_mask( Button1Mask, Val_Button1Mask )
    push_mask( Button2Mask, Val_Button2Mask )
    push_mask( Button3Mask, Val_Button3Mask )
    push_mask( Button4Mask, Val_Button4Mask )
    push_mask( Button5Mask, Val_Button5Mask )
    push_mask( ShiftMask,   Val_ShiftMask   )
    push_mask( LockMask,    Val_LockMask    )
    push_mask( ControlMask, Val_ControlMask )
    push_mask( Mod1Mask,    Val_Mod1Mask    )
    push_mask( Mod2Mask,    Val_Mod2Mask    )
    push_mask( Mod3Mask,    Val_Mod3Mask    )
    push_mask( Mod4Mask,    Val_Mod4Mask    )
    push_mask( Mod5Mask,    Val_Mod5Mask    )

    CAMLreturn(li);
}

/* KeyPress / KeyRelease */

CAMLprim value
ml_XKeyEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != KeyPress &&
        e->type != KeyRelease)
      caml_invalid_argument("not a key event");
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xkey.time > MAX_INT64)
      caml_failwith("xKeyEvent_datas: time value overflow");
#endif
    dat = caml_alloc(14, 0);
    Store_field( dat, 0, Val_ulong(e->xkey.serial) );
    Store_field( dat, 1, Val_bool(e->xkey.send_event) );
    Store_field( dat, 2, Val_Display(e->xkey.display) );
    Store_field( dat, 3, Val_Window(e->xkey.window) );
    Store_field( dat, 4, Val_Window(e->xkey.root) );
    Store_field( dat, 5, Val_Window(e->xkey.subwindow) );
    Store_field( dat, 6, Val_time(e->xkey.time) );
    Store_field( dat, 7, Val_int(e->xkey.x) );
    Store_field( dat, 8, Val_int(e->xkey.y) );
    Store_field( dat, 9, Val_int(e->xkey.x_root) );
    Store_field( dat, 10, Val_int(e->xkey.y_root) );
    Store_field( dat, 11, Val_state_mask(e->xkey.state) );
    Store_field( dat, 12, Val_KeyCode(e->xkey.keycode) );
    Store_field( dat, 13, Val_bool(e->xkey.same_screen) );
    CAMLreturn( dat );
}


#if 0

#define Val_Button1Mask  Val_int(0)
#define Val_Button2Mask  Val_int(1)
#define Val_Button3Mask  Val_int(2)
#define Val_Button4Mask  Val_int(3)
#define Val_Button5Mask  Val_int(4)

static inline value
Val_button_mask( unsigned int state_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    if (state_mask & Button1Mask) {
        cons = caml_alloc(2, 0);
        Store_field( cons, 0, Val_Button1Mask );
        Store_field( cons, 1, li );
        li = cons;
    }
    if (state_mask & Button2Mask) {
        cons = caml_alloc(2, 0);
        Store_field( cons, 0, Val_Button2Mask );
        Store_field( cons, 1, li );
        li = cons;
    }
    if (state_mask & Button3Mask) {
        cons = caml_alloc(2, 0);
        Store_field( cons, 0, Val_Button3Mask );
        Store_field( cons, 1, li );
        li = cons;
    }
    if (state_mask & Button4Mask) {
        cons = caml_alloc(2, 0);
        Store_field( cons, 0, Val_Button4Mask );
        Store_field( cons, 1, li );
        li = cons;
    }
    if (state_mask & Button5Mask) {
        cons = caml_alloc(2, 0);
        Store_field( cons, 0, Val_Button5Mask );
        Store_field( cons, 1, li );
        li = cons;
    }

    CAMLreturn(li);
}
#endif

/* MotionNotify */

CAMLprim value
ml_XMotionEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != MotionNotify)
      caml_invalid_argument("not a MotionNotify event");
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xmotion.time > MAX_INT64)
      caml_failwith("xMotionEvent_datas: time value overflow");
#endif
    dat = caml_alloc(14, 0);
    Store_field( dat, 0, Val_ulong(e->xmotion.serial) );
    Store_field( dat, 1, Val_bool(e->xmotion.send_event) );
    Store_field( dat, 2, Val_Display(e->xmotion.display) );
    Store_field( dat, 3, Val_Window(e->xmotion.window) );
    Store_field( dat, 4, Val_Window(e->xmotion.root) );
    Store_field( dat, 5, Val_Window(e->xmotion.subwindow) );
    Store_field( dat, 6, Val_time(e->xmotion.time) );
    Store_field( dat, 7, Val_int(e->xmotion.x) );
    Store_field( dat, 8, Val_int(e->xmotion.y) );
    Store_field( dat, 9, Val_int(e->xmotion.x_root) );
    Store_field( dat, 10, Val_int(e->xmotion.y_root) );
    Store_field( dat, 11, /*Val_button_mask*/Val_state_mask(e->xmotion.state) );
    Store_field( dat, 12, Val_char(e->xmotion.is_hint) );
    Store_field( dat, 13, Val_bool(e->xmotion.same_screen) );
    CAMLreturn( dat );
}

static const unsigned int button_table[] = {
    AnyButton,
    Button1,
    Button2,
    Button3,
    Button4,
    Button5,
};
    //unsigned int button = button_table[Long_val(v)];
#define Button_val(v) (button_table[Long_val(v)])

static inline value Val_button(unsigned int b)
{
    switch (b) {
        case AnyButton: return Val_int(0);
        case Button1: return Val_int(1);
        case Button2: return Val_int(2);
        case Button3: return Val_int(3);
        case Button4: return Val_int(4);
        case Button5: return Val_int(5);
    }
    return Val_int(0);
}

/* ButtonPress / ButtonRelease */

CAMLprim value
ml_XButtonEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ButtonPress &&
        e->type != ButtonRelease)
      caml_invalid_argument("not a MotionNotify event");
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xmotion.time > MAX_INT64)
      caml_failwith("xButtonEvent_datas: time value overflow");
#endif
    dat = caml_alloc(14, 0);
    Store_field( dat, 0, Val_ulong(e->xbutton.serial) );
    Store_field( dat, 1, Val_bool(e->xbutton.send_event) );
    Store_field( dat, 2, Val_Display(e->xbutton.display) );
    Store_field( dat, 3, Val_Window(e->xbutton.window) );
    Store_field( dat, 4, Val_Window(e->xbutton.root) );
    Store_field( dat, 5, Val_Window(e->xbutton.subwindow) );
    Store_field( dat, 6, Val_time(e->xbutton.time) );
    Store_field( dat, 7, Val_int(e->xbutton.x) );
    Store_field( dat, 8, Val_int(e->xbutton.y) );
    Store_field( dat, 9, Val_int(e->xbutton.x_root) );
    Store_field( dat, 10, Val_int(e->xbutton.y_root) );
    Store_field( dat, 11, Val_uint(e->xbutton.state) );
    Store_field( dat, 12, Val_button(e->xbutton.button) );
    Store_field( dat, 13, Val_bool(e->xbutton.same_screen) );
    CAMLreturn( dat );
}


static inline value Val_crossing_mode(int mode)
{
    switch (mode) {
        case NotifyNormal : return Val_int(0);
        case NotifyGrab   : return Val_int(1);
        case NotifyUngrab : return Val_int(2);
    }
    return Val_int(0);
}

static inline value Val_crossing_detail(int detail)
{
    switch (detail) {
        case NotifyAncestor         : return Val_int(0);
        case NotifyVirtual          : return Val_int(1);
        case NotifyInferior         : return Val_int(2);
        case NotifyNonlinear        : return Val_int(3);
        case NotifyNonlinearVirtual : return Val_int(4);
    }
    return Val_int(0);
}

static inline value Val_crossing_state(int state)
{
    switch (state) {
        case Button1Mask : return Val_int(0);
        case Button2Mask : return Val_int(1);
        case Button3Mask : return Val_int(2);
        case Button4Mask : return Val_int(3);
        case Button5Mask : return Val_int(4);
        case ShiftMask   : return Val_int(5);
        case LockMask    : return Val_int(6);
        case ControlMask : return Val_int(7);
        case Mod1Mask    : return Val_int(8);
        case Mod2Mask    : return Val_int(9);
        case Mod3Mask    : return Val_int(10);
        case Mod4Mask    : return Val_int(11);
        case Mod5Mask    : return Val_int(12);
    }
    return Val_int(0);
}

/* EnterNotify / LeaveNotify */

CAMLprim value
ml_XCrossingEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != EnterNotify &&
        e->type != LeaveNotify)
      caml_invalid_argument("not a Crossing event");
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xcrossing.time > MAX_INT64)
      caml_failwith("xCrossingEvent_datas: time value overflow");
#endif
    dat = caml_alloc(13, 0);
    Store_field( dat,  0, Val_Window(e->xcrossing.window) );
    Store_field( dat,  1, Val_Window(e->xcrossing.root) );
    Store_field( dat,  2, Val_Window(e->xcrossing.subwindow) );
    Store_field( dat,  3, Val_time(e->xcrossing.time) );
    Store_field( dat,  4, Val_int(e->xcrossing.x) );
    Store_field( dat,  5, Val_int(e->xcrossing.y) );
    Store_field( dat,  6, Val_int(e->xcrossing.x_root) );
    Store_field( dat,  7, Val_int(e->xcrossing.y_root) );
    Store_field( dat,  8, Val_crossing_mode(e->xcrossing.mode) );
    Store_field( dat,  9, Val_crossing_detail(e->xcrossing.detail) );
    Store_field( dat, 10, Val_bool(e->xcrossing.same_screen) );
    Store_field( dat, 11, Val_bool(e->xcrossing.focus) );
    Store_field( dat, 12, Val_crossing_state(e->xcrossing.state) );
    CAMLreturn( dat );
}


static inline value Val_focus_mode(int n)
{
    switch (n) {
        case NotifyNormal      : return Val_int(0);
        case NotifyGrab        : return Val_int(1);
        case NotifyUngrab      : return Val_int(2);
        case NotifyWhileGrabbed: return Val_int(3);
    }
    return Val_int(0);
}

static inline value Val_focus_detail(int d)
{
    switch (d) {
        case NotifyAncestor        : return Val_int(0);
        case NotifyVirtual         : return Val_int(1);
        case NotifyInferior        : return Val_int(2);
        case NotifyNonlinear       : return Val_int(3);
        case NotifyNonlinearVirtual: return Val_int(4);
        case NotifyPointer         : return Val_int(5);
        case NotifyPointerRoot     : return Val_int(6);
        case NotifyDetailNone      : return Val_int(7);
    }
    return Val_int(0);
}

/* FocusIn / FocusOut */

CAMLprim value
ml_XFocusChangeEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != FocusIn &&
        e->type != FocusOut)
      caml_invalid_argument("not a FocusChange event");
#endif
    dat = caml_alloc(2, 0);
    Store_field( dat, 0, Val_focus_mode(e->xfocus.mode) );
    Store_field( dat, 1, Val_focus_detail(e->xfocus.detail) );
    CAMLreturn( dat );
}

/* KeymapNotify */

CAMLprim value
ml_XKeymapEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal2( dat, key_str );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != KeymapNotify)
      caml_invalid_argument("not a KeymapNotify event");
#endif
    key_str = caml_alloc_string (32);
    memcpy( String_val(key_str), (e->xkeymap.key_vector), 32 );

    dat = caml_alloc(1, 0);
    Store_field( dat, 0, key_str );
    CAMLreturn( dat );
}

/* Expose */

CAMLprim value
ml_XExposeEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != Expose)
      caml_invalid_argument("not an Expose event");
#endif
    dat = caml_alloc(5, 0);
    Store_field( dat, 0, Val_int(e->xexpose.x) );
    Store_field( dat, 1, Val_int(e->xexpose.y) );
    Store_field( dat, 2, Val_int(e->xexpose.width) );
    Store_field( dat, 3, Val_int(e->xexpose.height) );
    Store_field( dat, 4, Val_int(e->xexpose.count) );
    CAMLreturn( dat );
}

/*
   TODO  GraphicsExpose
   TODO  NoExpose
*/

static inline value Val_visibility_state(int v)
{
    switch (v) {
        case VisibilityUnobscured       : return Val_int(0);
        case VisibilityPartiallyObscured: return Val_int(1);
        case VisibilityFullyObscured    : return Val_int(2);
    }
    return Val_int(0);
}

/* VisibilityNotify */

CAMLprim value
ml_XVisibilityEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != VisibilityNotify)
      caml_invalid_argument("not a VisibilityNotify event");
#endif
    dat = caml_alloc(1, 0);
    Store_field( dat, 0, Val_visibility_state(e->xvisibility.state) );
    CAMLreturn( dat );
}

/* CreateNotify */

CAMLprim value
ml_XCreateWindowEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e = XEvent_val(event);
    dat = caml_alloc(11, 0);
    Store_field( dat, 0, Val_ulong(e->xcreatewindow.serial) );
    Store_field( dat, 1, Val_bool(e->xcreatewindow.send_event) );
    Store_field( dat, 2, Val_Display(e->xcreatewindow.display) );
    Store_field( dat, 3, Val_Window(e->xcreatewindow.parent) );
    Store_field( dat, 4, Val_Window(e->xcreatewindow.window) );
    Store_field( dat, 5, Val_int(e->xcreatewindow.x) );
    Store_field( dat, 6, Val_int(e->xcreatewindow.y) );
    Store_field( dat, 7, Val_int(e->xcreatewindow.width) );
    Store_field( dat, 8, Val_int(e->xcreatewindow.height) );
    Store_field( dat, 9, Val_int(e->xcreatewindow.border_width) );
    Store_field( dat, 10, Val_bool(e->xcreatewindow.override_redirect) );
    CAMLreturn( dat );
}

/* DestroyNotify */

CAMLprim value
ml_XDestroyWindowEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != DestroyNotify)
      caml_invalid_argument("not a DestroyNotify event");
#endif
    dat = caml_alloc(2, 0);
    Store_field( dat, 0, Val_Window(e->xdestroywindow.event) );
    Store_field( dat, 1, Val_Window(e->xdestroywindow.window) );
    CAMLreturn( dat );
}

/*
   TODO  UnmapNotify
   TODO  MapNotify
   TODO  MapRequest
*/

/* ReparentNotify */

CAMLprim value
ml_XReparentEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ReparentNotify)
      caml_invalid_argument("not a ReparentNotify event");
#endif
    dat = caml_alloc(2, 0);
    Store_field( dat, 0, Val_Window(e->xreparent.event) );
    Store_field( dat, 1, Val_Window(e->xreparent.window) );
    Store_field( dat, 2, Val_Window(e->xreparent.parent) );
    Store_field( dat, 3, Val_int(e->xreparent.x) );
    Store_field( dat, 4, Val_int(e->xreparent.y) );
    Store_field( dat, 5, Val_bool(e->xreparent.override_redirect) );
    CAMLreturn( dat );
}


/* ConfigureNotify */

CAMLprim value
ml_XConfigureEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ConfigureNotify)
      caml_invalid_argument("not a ConfigureNotify event");
#endif
    dat = caml_alloc(7, 0);
    Store_field( dat, 0, Val_int(e->xconfigure.x) );
    Store_field( dat, 1, Val_int(e->xconfigure.y) );
    Store_field( dat, 2, Val_int(e->xconfigure.width) );
    Store_field( dat, 3, Val_int(e->xconfigure.height) );
    Store_field( dat, 4, Val_int(e->xconfigure.border_width) );
    Store_field( dat, 5, Val_Window(e->xconfigure.above) );
    Store_field( dat, 6, Val_bool(e->xconfigure.override_redirect) );
    CAMLreturn( dat );
}

/* ConfigureRequest */

static inline value Val_xconfreq_detail(int detail)
{
    switch (detail) {
        case Above    : return Val_int(0);
        case Below    : return Val_int(1);
        case TopIf    : return Val_int(2);
        case BottomIf : return Val_int(3);
        case Opposite : return Val_int(4);
    }
    return Val_int(0);
}

CAMLprim value
ml_XConfigureRequestEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ConfigureRequest)
      caml_invalid_argument("not a ConfigureRequest event");
#endif
    dat = caml_alloc(10, 0);
    Store_field( dat, 0, Val_Window(e->xconfigurerequest.parent) );
    Store_field( dat, 1, Val_Window(e->xconfigurerequest.window) );
    Store_field( dat, 2, Val_int(e->xconfigurerequest.x) );
    Store_field( dat, 3, Val_int(e->xconfigurerequest.y) );
    Store_field( dat, 4, Val_int(e->xconfigurerequest.width) );
    Store_field( dat, 5, Val_int(e->xconfigurerequest.height) );
    Store_field( dat, 6, Val_int(e->xconfigurerequest.border_width) );
    Store_field( dat, 7, Val_Window(e->xconfigurerequest.above) );
    Store_field( dat, 8, Val_xconfreq_detail(e->xconfigurerequest.detail) );
    Store_field( dat, 9, Val_ulong(e->xconfigurerequest.value_mask) );  /* TODO */
    CAMLreturn( dat );
}


/*
   TODO  GravityNotify
*/

/* ResizeRequest */

CAMLprim value
ml_XResizeRequestEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ResizeRequest)
      caml_invalid_argument("not a ResizeRequest event");
#endif
    dat = caml_alloc(2, 0);
    Store_field( dat, 0, Val_int(e->xresizerequest.width) );
    Store_field( dat, 1, Val_int(e->xresizerequest.height) );
    CAMLreturn( dat );
}

/*
   TODO  CirculateNotify
   TODO  CirculateRequest
   TODO  PropertyNotify
   TODO  SelectionClear
   TODO  SelectionRequest
*/


CAMLprim value
Val_Atom_option( Atom a )
{
    if (a == None) return Val_none;
    else return Val_some( Val_Atom(a) );
}

/* SelectionNotify */

CAMLprim value
ml_XSelectionEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != SelectionNotify)
      caml_invalid_argument("not a SelectionNotify event");
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xselection.time > MAX_INT64)
      caml_failwith("xSelectionEvent_datas: time value overflow");
#endif
    dat = caml_alloc(5, 0);
    Store_field( dat, 0, Val_Window(e->xselection.requestor) );
    Store_field( dat, 1, Val_Atom(e->xselection.selection) );
    Store_field( dat, 2, Val_Atom(e->xselection.target) );
    Store_field( dat, 3, Val_Atom_option(e->xselection.property) );  /* Atom or None */
    Store_field( dat, 4, Val_time(e->xselection.time) );
    CAMLreturn( dat );
}


/*
   TODO  ColormapNotify
   TODO  ClientMessage
   TODO  MappingNotify
*/


/* XErrorEvent */

CAMLprim value
ml_XErrorEvent_datas( value event )
{
    CAMLparam1( event );
    CAMLlocal1( dat );
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    /*
    // don't know how is set this field for the XerrorEvent
    if (e->type != ??? )
      caml_invalid_argument("not an error event");
    */
#endif
#if CHECK_TIME_OVERFLOW
    if (e->xkey.time > MAX_INT64)
      caml_failwith("xKeyEvent_datas: time value overflow");
#endif
    dat = caml_alloc(6, 0);
    Store_field( dat, 0, Val_Display(e->xerror.display) );
    Store_field( dat, 1, Val_XID(e->xerror.resourceid ) );
    Store_field( dat, 2, Val_ulong(e->xerror.serial ) );
    Store_field( dat, 3, Val_char(e->xerror.error_code ) );
    Store_field( dat, 4, Val_char(e->xerror.request_code ) );
    Store_field( dat, 5, Val_char(e->xerror.minor_code ) );
    CAMLreturn( dat );
}

/* }}} */


CAMLprim value
ml_XSendEvent(
        value dpy,
        value win,
        value propagate,
        value event_mask,
        value event_content )
{
    XEvent ev;

    value cont = Field(event_content,0);
    switch (Tag_val(event_content))
    {
        case 0:   // {{{ XMotionEvCnt 
            ev.type = MotionNotify;
            ev.xmotion.serial      = ULong_val(Field(cont, 0));
            ev.xmotion.send_event  = Bool_val(Field(cont, 1));
            ev.xmotion.display     = Display_val(Field(cont, 2));
            ev.xmotion.window      = Window_val(Field(cont, 3));
            ev.xmotion.root        = Window_val(Field(cont, 4));
            ev.xmotion.subwindow   = Window_val(Field(cont, 5));
            ev.xmotion.time        = Time_val(Field(cont, 6));
            ev.xmotion.x           = Int_val(Field(cont, 7));
            ev.xmotion.y           = Int_val(Field(cont, 8));
            ev.xmotion.x_root      = Int_val(Field(cont, 9));
            ev.xmotion.y_root      = Int_val(Field(cont, 10));
            ev.xmotion.state       = State_mask_val(Field(cont, 11));
            ev.xmotion.is_hint     = Char_val(Field(cont, 12));
            ev.xmotion.same_screen = Bool_val(Field(cont, 13));
            break; // }}}
        case 1:   // {{{ XKeyPressedEvCnt 
            ev.type = KeyPress;
            ev.xkey.serial      = ULong_val(Field(cont, 0));
            ev.xkey.send_event  = Bool_val(Field(cont, 1));
            ev.xkey.display     = Display_val(Field(cont, 2));
            ev.xkey.window      = Window_val(Field(cont, 3));
            ev.xkey.root        = Window_val(Field(cont, 4));
            ev.xkey.subwindow   = Window_val(Field(cont, 5));
            ev.xkey.time    = Time_val(Field(cont, 6));
            ev.xkey.x       = Int_val(Field(cont, 7));
            ev.xkey.y       = Int_val(Field(cont, 8));
            ev.xkey.x_root  = Int_val(Field(cont, 9));
            ev.xkey.y_root  = Int_val(Field(cont, 10));
            ev.xkey.state   = State_mask_val(Field(cont, 11));
            ev.xkey.keycode = KeyCode_val(Field(cont, 12));
            ev.xkey.same_screen = Bool_val(Field(cont, 13));
            break; // }}}
        case 2:   // {{{ XKeyReleasedEvCnt 
            ev.type = KeyRelease;
            ev.xkey.serial      = ULong_val(Field(cont, 0));
            ev.xkey.send_event  = Bool_val(Field(cont, 1));
            ev.xkey.display     = Display_val(Field(cont, 2));
            ev.xkey.window      = Window_val(Field(cont, 3));
            ev.xkey.root        = Window_val(Field(cont, 4));
            ev.xkey.subwindow   = Window_val(Field(cont, 5));
            ev.xkey.time    = Time_val(Field(cont, 6));
            ev.xkey.x       = Int_val(Field(cont, 7));
            ev.xkey.y       = Int_val(Field(cont, 8));
            ev.xkey.x_root  = Int_val(Field(cont, 9));
            ev.xkey.y_root  = Int_val(Field(cont, 10));
            ev.xkey.state   = State_mask_val(Field(cont, 11));
            ev.xkey.keycode = KeyCode_val(Field(cont, 12));
            ev.xkey.same_screen = Bool_val(Field(cont, 13));
            break; // }}}
        case 3:   // {{{ XButtonPressedEvCnt 
            ev.type = ButtonPress;
            ev.xbutton.serial      = ULong_val(Field(cont, 0));
            ev.xbutton.send_event  = Bool_val(Field(cont, 1));
            ev.xbutton.display     = Display_val(Field(cont, 2));
            ev.xbutton.window      = Window_val(Field(cont, 3));
            ev.xbutton.root        = Window_val(Field(cont, 4));
            ev.xbutton.subwindow   = Window_val(Field(cont, 5));
            ev.xbutton.time        = Time_val(Field(cont, 6));
            ev.xbutton.x           = Int_val(Field(cont, 7));
            ev.xbutton.y           = Int_val(Field(cont, 8));
            ev.xbutton.x_root      = Int_val(Field(cont, 9));
            ev.xbutton.y_root      = Int_val(Field(cont, 10));
            ev.xbutton.state       = UInt_val(Field(cont, 11));
            ev.xbutton.button      = Button_val(Field(cont, 12));
            ev.xbutton.same_screen = Bool_val(Field(cont, 13));
            break; // }}}
        case 4:   // {{{ XButtonReleasedEvCnt 
            ev.type = ButtonRelease;
            ev.xbutton.serial      = ULong_val(Field(cont, 0));
            ev.xbutton.send_event  = Bool_val(Field(cont, 1));
            ev.xbutton.display     = Display_val(Field(cont, 2));
            ev.xbutton.window      = Window_val(Field(cont, 3));
            ev.xbutton.root        = Window_val(Field(cont, 4));
            ev.xbutton.subwindow   = Window_val(Field(cont, 5));
            ev.xbutton.time        = Time_val(Field(cont, 6));
            ev.xbutton.x           = Int_val(Field(cont, 7));
            ev.xbutton.y           = Int_val(Field(cont, 8));
            ev.xbutton.x_root      = Int_val(Field(cont, 9));
            ev.xbutton.y_root      = Int_val(Field(cont, 10));
            ev.xbutton.state       = UInt_val(Field(cont, 11));
            ev.xbutton.button      = Button_val(Field(cont, 12));
            ev.xbutton.same_screen = Bool_val(Field(cont, 13));
            break; // }}}
        case 5:   // XCrossingEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 6:   // XFocusChangeEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 7:   // XKeymapEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 8:   // XExposeEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 9:   // XGraphicsExposeEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 10:  // XNoExposeEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 11:  // XVisibilityEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 12:  // XCreateWindowEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 13:  // XDestroyWindowEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 14:  // XUnmapEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 15:  // XMapEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 16:  // XMapRequestEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 17:  // XReparentEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 18:  // XConfigureEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 19:  // XConfigureRequestEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 20:  // XGravityEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 21:  // XResizeRequestEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 22:  // XCirculateEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 23:  // XCirculateRequestEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 24:  // XPropertyEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 25:  // XSelectionClearEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 26:  // XSelectionRequestEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 27:  // XSelectionEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 28:  // XColormapEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 29:  // XClientMessageEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        case 30:  // XMappingEvCnt
            caml_failwith("xSendEvent TODO: this event_content is not handled yet");
            break;
        default: caml_failwith("variant handling bug");
    }

    Status st = XSendEvent(
        Display_val(dpy),
        Window_val(win),
        Bool_val(propagate),
        event_mask_table[Long_val(event_mask)],
        &ev
    );
    if (st == 0) {
      caml_failwith("xSendEvent: failed");
    }
    return Val_unit;
}


CAMLprim value
ml_XEvent_xclient_data_l_0( value event )
{
    XEvent * e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    // The type inference should garanty this:
    if (e->type != ClientMessage)
      caml_invalid_argument("not a ClientMessage event");
#endif
    long atom = e->xclient.data.l[0];
    return Val_Atom( atom );
}

/*
int XLookupString(
    XKeyEvent*       event_struct,
    char*            buffer_return,
    int              bytes_buffer,
    KeySym*          keysym_return,
    XComposeStatus*  status_in_out );
*/
#if 0
#include <X11/keysym.h>
CAMLprim value
_ml_XLookupString( value event )
{
    XEvent * e;
    KeySym keysym;
    e = XEvent_val(event);
    XComposeStatus*  status_in_out = NULL;
    char buffer_txt[256];
    char * p;
    int nchar;
    nchar = XLookupString(
        &(e->xkey),
        buffer_txt, sizeof(buffer_txt),
        &keysym,
        status_in_out );
    for (p = buffer_txt; nchar > 0; ++p, --nchar)
        printf("%c", *p);
    printf("\n"); fflush(stdout);
    switch (keysym) {
#define KEY_CASE(k)  case k: printf(#k "\n"); break;
// {{{
KEY_CASE(XK_KP_F1)
KEY_CASE(XK_F1)
KEY_CASE(XK_a)
KEY_CASE(XK_b)
KEY_CASE(XK_c)
KEY_CASE(XK_d)
KEY_CASE(XK_e)
KEY_CASE(XK_f)
KEY_CASE(XK_g)
KEY_CASE(XK_h)
KEY_CASE(XK_i)
KEY_CASE(XK_j)
KEY_CASE(XK_A)
KEY_CASE(XK_B)
KEY_CASE(XK_C)
KEY_CASE(XK_D)
KEY_CASE(XK_E)
KEY_CASE(XK_F)
KEY_CASE(XK_G)
// }}}
#undef KEY_CASE
    }
    return Val_unit;
}
#endif

CAMLprim value
ml_XLookupString( value event, value buffer )
{
    CAMLparam1( event );
    CAMLlocal1( ret );
    KeySym keysym;
    XEvent * e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    if (e->type != KeyPress &&
        e->type != KeyRelease)
      caml_invalid_argument("not a key event");
#endif
    // XComposeStatus stat;
    int nchars = XLookupString(  // from <X11/Xutil.h>
        &(e->xkey),
        String_val(buffer),
        caml_string_length(buffer),
        &keysym,
        NULL /* &stat */ );
    ret = caml_alloc(2, 0);
    Store_field( ret, 0, Val_int(nchars) );
    Store_field( ret, 1, Val_keysym(keysym) );
    CAMLreturn( ret );
}

// TODO: Xutf8LookupString, XwcLookupString, XmbLookupString


CAMLprim value
ml_XLookupKeysym( value event, value index )
{
    XEvent * e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    if (e->type != KeyPress &&
        e->type != KeyRelease)
      caml_invalid_argument("not a key event");
#endif
    KeySym keysym = XLookupKeysym(
        &(e->xkey),
        Int_val(index) );  // index from 0 to (keysyms_per_keycode - 1)
    return Val_keysym(keysym);
}


CAMLprim value
ml_XRefreshKeyboardMapping( value event )
{
    XEvent *e;
    e = XEvent_val(event);
#if CHECK_EVENT_TYPE
    if (e->type != MappingNotify)
      caml_invalid_argument("not a MappingNotify event");
#endif
    XRefreshKeyboardMapping(&(e->xmapping));
    return Val_unit;
}

CAMLprim value
ml_XDisplayKeycodes( value dpy )
{
    CAMLlocal1( tpl );
    int min_keycodes, max_keycodes;
    //GET_STATUS
    XDisplayKeycodes(
        Display_val(dpy),
        &min_keycodes,
        &max_keycodes
    );
    //CHECK_STATUS(XDisplayKeycodes,1);
    tpl = caml_alloc(2, 0);
    Store_field( tpl, 0, Val_KeyCode(min_keycodes) );
    Store_field( tpl, 1, Val_KeyCode(max_keycodes) );
    return tpl;
}

/*
TODO:

    //GET_STATUS
    XRebindKeysym(
        Display_val(dpy),
        KeySym         keysym,
        KeySym*        list,
        int            mod_count,
        _Xconst unsigned char*  string,
        int            bytes_string
    );
    //CHECK_STATUS(XRebindKeysym,0);

http://tronche.com/gui/x/xlib/utilities/XRebindKeysym.html

*/

CAMLprim value
ml_XGetKeyboardMapping( value dpy, value first_keycode, value ml_keycode_count )
{
    CAMLparam3(dpy, first_keycode, ml_keycode_count);
    CAMLlocal2(ml_keysyms, sub);
    int keysyms_per_keycode;
    KeySym *keysyms;
    //unsigned int keysyms_len;
    int i, j;
    int keycode_count = Int_val(ml_keycode_count);

    {
        int min_keycode, max_keycode;
        XDisplayKeycodes(
            Display_val(dpy),
            &min_keycode,
            &max_keycode
        );
        if (Long_val(first_keycode) < min_keycode)
            caml_invalid_argument("xGetKeyboardMapping: "
              "first_keycode must be greater than or equal to min_keycode");
    }

    keysyms = XGetKeyboardMapping(
        Display_val(dpy),
        KeyCode_val(first_keycode),
        keycode_count,
        &keysyms_per_keycode
    );

    // number of elements in the KeySyms list
    //keysyms_len = (keycode_count * keysyms_per_keycode);

    ml_keysyms = caml_alloc(keycode_count, 0);

    printf("keysyms_per_keycode = %d\n", keysyms_per_keycode); // XXX DEBUG XXX

    for (i=0; i < keycode_count; ++i)
    {
        int n = 0;
        for (j=0; j < keysyms_per_keycode; ++j)
        {
            // don't return undefined keysyms
            if (keysyms[keysyms_per_keycode + i + j] != NoSymbol) n++;
        }
        sub = caml_alloc(n, 0);
        for (j=0; j < n; ++j)
        {
            Store_field(sub, j,
                Val_keysym( keysyms[keysyms_per_keycode + i + j] )
            );
        }
        Store_field(ml_keysyms, i, sub);
    }

    XFree(keysyms);
    CAMLreturn( ml_keysyms );
}
// http://tronche.com/gui/x/xlib/input/XGetKeyboardMapping.html

/*
TODO:

    KeySym keysym = XStringToKeysym( char* string );
*/


CAMLprim value
ml_XChangeKeyboardMapping(  // ---------------- WIP
        value dpy,
        value first_keycode,
        value keysyms_per_keycode,
        value keysyms_arr,
        value num_codes )
{
    KeySym* keysyms;
    int i, keysyms_len;
    int keysyms_len_should;
    keysyms_len = Wosize_val(keysyms_arr);
    keysyms_len_should = Int_val(num_codes) * Int_val(keysyms_per_keycode);
    if (keysyms_len != keysyms_len_should) {
        caml_invalid_argument("xChangeKeyboardMapping: "
                              "keysyms array should contain "
                              "(num_codes * keysyms_per_keycode) elements");
    }
    /* From the man: http://tronche.com/gui/x/xlib/input/XChangeKeyboardMapping.html
 
       The specified first_keycode must be greater than or equal to min_keycode
       returned by XDisplayKeycodes(), or a BadValue error results.
       In addition, the following expression must be less than or equal to max_keycode,
       or a BadValue error results: (first_keycode + num_codes - 1)
 
       TODO: handle this BadValue error */
    {
        int min_keycode, max_keycode;
        XDisplayKeycodes(
            Display_val(dpy),
            &min_keycode,
            &max_keycode
        );
        if (Long_val(first_keycode) < min_keycode) {
            caml_invalid_argument("xChangeKeyboardMapping: "
              "first_keycode must be greater than or equal to min_keycode");
        }
        if ((Long_val(first_keycode) + Int_val(num_codes) - 1) > max_keycode) {
            caml_invalid_argument("xChangeKeyboardMapping: "
              "(first_keycode + num_codes - 1) must be less than or equal to max_keycode");
        }
    }
    keysyms = malloc(keysyms_len * sizeof(KeySym));
    for (i=0; i < keysyms_len; ++i)
    {
        keysyms[i] = Keysym_val(Field(keysyms_arr, i));
    }
    //GET_STATUS
    XChangeKeyboardMapping(
        Display_val(dpy),
        KeyCode_val(first_keycode),
        Int_val(keysyms_per_keycode),
        keysyms,
        Int_val(num_codes)
    );
    free(keysyms);
    //CHECK_STATUS(XChangeKeyboardMapping,0);
    return Val_unit;
}


CAMLprim value
ml_XChangeKeyboardMapping_single(
        value dpy,
        value first_keycode,
        value keysym )
{
    KeySym keysyms[1];
    keysyms[0] = Keysym_val(keysym);
    //GET_STATUS
    XChangeKeyboardMapping(
        Display_val(dpy),
        KeyCode_val(first_keycode),
        1, keysyms, 1
    );
    //CHECK_STATUS(XChangeKeyboardMapping,0);
    return Val_unit;
}


/* Keyboard */

CAMLprim value
ml_XAutoRepeatOff( value dpy )
{
    GET_STATUS XAutoRepeatOff( Display_val(dpy) );
    CHECK_STATUS(XAutoRepeatOff,1);
    return Val_unit;
}

CAMLprim value
ml_XAutoRepeatOn( value dpy )
{
    GET_STATUS XAutoRepeatOn( Display_val(dpy) );
    CHECK_STATUS(XAutoRepeatOn,1);
    return Val_unit;
}

CAMLprim value
ml_XQueryKeymap( value dpy )
{
    CAMLparam1( dpy );
    CAMLlocal1( ml_keys );
    ml_keys = caml_alloc_string(32);
    char *keys_ptr;
    keys_ptr = String_val(ml_keys);
    GET_STATUS XQueryKeymap(
        Display_val(dpy),
        keys_ptr );
    CHECK_STATUS(XQueryKeymap,1);
    CAMLreturn( ml_keys );
}


CAMLprim value
ml_XQueryPointer( value dpy, value win )
{
    CAMLparam2( dpy, win );
    CAMLlocal2( pntr, subp );
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    Bool b = XQueryPointer(
        Display_val(dpy),
        Window_val(win),
        &root,
        &child,
        &root_x,
        &root_y,
        &win_x,
        &win_y,
        &mask
    );
    pntr = caml_alloc(5, 0);
    if (b) {
        subp = caml_alloc(3, 0);
        Store_field( subp, 0, Val_Window(child) );
        Store_field( subp, 1, Val_int(win_x) );
        Store_field( subp, 2, Val_int(win_y) );
 
        Store_field( pntr, 0, Val_Window(root) );
        Store_field( pntr, 1, Val_int(root_x) );
        Store_field( pntr, 2, Val_int(root_y) );
        Store_field( pntr, 3, Val_some( subp ) );
        Store_field( pntr, 4, Val_state_mask(mask) );
    } else {
        Store_field( pntr, 0, Val_Window(root) );
        Store_field( pntr, 1, Val_int(root_x) );
        Store_field( pntr, 2, Val_int(root_y) );
        Store_field( pntr, 3, Val_none );
        Store_field( pntr, 4, Val_state_mask(mask) );
    }
    CAMLreturn( pntr );
}

CAMLprim value
ml_XGetKeyboardControl( value dpy )
{
    CAMLparam1( dpy );
    CAMLlocal2( tpl, ml_auto_repeats );
    XKeyboardState kbs;
    GET_STATUS  XGetKeyboardControl(
        Display_val(dpy),
        &kbs );
    CHECK_STATUS(XGetKeyboardControl,1);
    tpl = caml_alloc(7, 0);
    ml_auto_repeats = caml_alloc_string(32);
    memcpy(String_val(ml_auto_repeats), kbs.auto_repeats, 32);
    Store_field( tpl, 0, Val_int(kbs.key_click_percent) );
    Store_field( tpl, 1, Val_int(kbs.bell_percent) );
    Store_field( tpl, 2, Val_uint(kbs.bell_pitch) );
    Store_field( tpl, 3, Val_uint(kbs.bell_duration) );
    Store_field( tpl, 4, Val_ulong(kbs.led_mask) );  // TODO: WRAP ME
    Store_field( tpl, 5, Val_int( (kbs.global_auto_repeat == AutoRepeatModeOff ? 0 : 1)) );
    Store_field( tpl, 6, ml_auto_repeats );
    CAMLreturn( tpl );
}

#if 0
static const unsigned int keyboardcontrol_table[] = {
    KBKeyClickPercent,
    KBBellPercent,
    KBBellPitch,
    KBBellDuration,
    KBLed,
    KBLedMode,
    KBKey,
    KBAutoRepeatMode,
};
#define keyboardcontrol_mask_val(i) (keyboardcontrol_table[Long_val(i)])

/* TODO

    if (mask & KBKeyClickPercent)  *value++ = value_list->key_click_percent;
    if (mask & KBBellPercent)      *value++ = value_list->bell_percent;
    if (mask & KBBellPitch)        *value++ = value_list->bell_pitch;
    if (mask & KBBellDuration)     *value++ = value_list->bell_duration;
    if (mask & KBLed)              *value++ = value_list->led;
    if (mask & KBLedMode)          *value++ = value_list->led_mode;
    if (mask & KBKey)              *value++ = value_list->key;
    if (mask & KBAutoRepeatMode)   *value++ = value_list->auto_repeat_mode;
typedef struct {
        int key_click_percent;
        int bell_percent;
        int bell_pitch; 
        int bell_duration;
        int led;
        int led_mode;
        int key;        
        int auto_repeat_mode;   // On, Off, Default
} XKeyboardControl;
*/

CAMLprim value
ml_XChangeKeyboardControl( value dpy, value ml_xkeyboardcontrol_tpl )
{
    unsigned long value_mask;
    while ( em_list != Val_emptylist )
    {
        value head = Field(em_list, 0);
        long mask = keyboardcontrol_mask_val(head);
        value_mask |= mask;
        em_list = Field(em_list, 1);
    }

    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        value_mask,
        XKeyboardControl*  values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}
#endif

CAMLprim value
ml_XChangeKeyboardControl_bell_percent( value dpy, value ml_bell_percent )
{
    XKeyboardControl  keyboard_control_values;
    keyboard_control_values.bell_percent = Int_val(ml_bell_percent);
    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        KBBellPercent,
        &keyboard_control_values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}

CAMLprim value
ml_XChangeKeyboardControl_bell_pitch( value dpy, value ml_bell_pitch )
{
    XKeyboardControl  keyboard_control_values;
    keyboard_control_values.bell_pitch = Int_val(ml_bell_pitch);
    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        KBBellPitch,
        &keyboard_control_values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}

CAMLprim value
ml_XChangeKeyboardControl_bell_duration( value dpy, value ml_bell_duration )
{
    XKeyboardControl  keyboard_control_values;
    keyboard_control_values.bell_duration = Int_val(ml_bell_duration);
    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        KBBellDuration,
        &keyboard_control_values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}

CAMLprim value
ml_XChangeKeyboardControl_bell(
        value dpy,
        value ml_bell_percent,
        value ml_bell_pitch,
        value ml_bell_duration )
{
    XKeyboardControl  keyboard_control_values;
    keyboard_control_values.bell_percent = Int_val(ml_bell_percent);
    keyboard_control_values.bell_pitch = Int_val(ml_bell_pitch);
    keyboard_control_values.bell_duration = Int_val(ml_bell_duration);
    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        KBBellPercent &
        KBBellPitch &
        KBBellDuration,
        &keyboard_control_values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}

CAMLprim value
ml_XChangeKeyboardControl_key_click_percent( value dpy, value ml_key_click_percent )
{
    XKeyboardControl  keyboard_control_values;
    keyboard_control_values.key_click_percent = Int_val(ml_key_click_percent);
    //GET_STATUS
    XChangeKeyboardControl(
        Display_val(dpy),
        KBKeyClickPercent,
        &keyboard_control_values
    );
    //CHECK_STATUS(XChangeKeyboardControl,1);
    return Val_unit;
}

/* TODO: place */
CAMLprim value
ml_XChangePointerControl(
        value dpy,
        value do_accel,
        value do_threshold,
        value accel_numerator,
        value accel_denominator,
        value threshold )
{
    //GET_STATUS
    XChangePointerControl(
        Display_val(dpy),
        Bool_val(do_accel),
        Bool_val(do_threshold),
        Int_val(accel_numerator),
        Int_val(accel_denominator),
        Int_val(threshold)
    );
    //CHECK_STATUS(XChangePointerControl,1);
    return Val_unit;
}
CAMLprim value
ml_XChangePointerControl_bytecode( value * argv, int argn )
{

    return ml_XChangePointerControl( argv[0], argv[1], argv[2],
                                     argv[3], argv[4], argv[5] );
}

CAMLprim value
ml_XGetPointerControl( value dpy )
{
    CAMLparam1(dpy);
    CAMLlocal1( pnt_ctrl );
    int accel_numerator;
    int accel_denominator;
    int threshold;
    //GET_STATUS
    XGetPointerControl(
        Display_val(dpy),
        &accel_numerator,
        &accel_denominator,
        &threshold
    );
    //CHECK_STATUS(XGetPointerControl, TODO );
    pnt_ctrl = caml_alloc(3, 0);
    Store_field( pnt_ctrl, 0, Val_int(accel_numerator) );
    Store_field( pnt_ctrl, 1, Val_int(accel_denominator) );
    Store_field( pnt_ctrl, 2, Val_int(threshold) );
    CAMLreturn( pnt_ctrl );
}


/* ScreenSaver */

CAMLprim value
ml_XForceScreenSaver( value dpy, value mode )
{
    GET_STATUS  XForceScreenSaver(
        Display_val(dpy),
        ( Long_val(mode) ? ScreenSaverReset : ScreenSaverActive )
    );
    CHECK_STATUS(XForceScreenSaver,1);
    return Val_unit;
}

static const int prefer_blanking_table[] = {
    DontPreferBlanking,
    PreferBlanking,
    DefaultBlanking
};
#define Prefer_blanking_val(i) (prefer_blanking_table[Long_val(i)])
static inline value Val_prefer_blanking(int v) {
    switch (v) {
        case DontPreferBlanking:  return Val_int(0);
        case PreferBlanking:      return Val_int(1);
        case DefaultBlanking:     return Val_int(2);
    }
    return Val_int(0);
}

static const int allow_exposures_table[] = {
    DontAllowExposures,
    AllowExposures,
    DefaultExposures
};
#define Allow_exposures_val(i) (allow_exposures_table[Long_val(i)])
static inline value Val_allow_exposures(int v) {
    switch (v) {
        case DontAllowExposures:  return Val_int(0);
        case AllowExposures:      return Val_int(1);
        case DefaultExposures:    return Val_int(2);
    }
    return Val_int(0);
}

CAMLprim value
ml_XGetScreenSaver( value dpy )
{
    CAMLparam1( dpy );
    CAMLlocal1( tpl );
    int timeout;
    int interval;
    int prefer_blanking;
    int allow_exposures;
    GET_STATUS  XGetScreenSaver(
        Display_val(dpy),
        &timeout,
        &interval,
        &prefer_blanking,
        &allow_exposures
    );
    CHECK_STATUS(XGetScreenSaver,1);
    tpl = caml_alloc(4, 0);
    Store_field( tpl, 0, Val_int(timeout) );
    Store_field( tpl, 1, Val_int(interval) );
    Store_field( tpl, 2, Val_prefer_blanking(prefer_blanking) );
    Store_field( tpl, 3, Val_allow_exposures(allow_exposures) );
    CAMLreturn( tpl );
}

CAMLprim value
ml_XSetScreenSaver(
        value dpy,
        value timeout,
        value interval,
        value prefer_blanking,
        value allow_exposures )
{
    //GET_STATUS
    XSetScreenSaver(
        Display_val(dpy),
        Int_val(timeout),
        Int_val(interval),
        Prefer_blanking_val(prefer_blanking),
        Allow_exposures_val(allow_exposures)
    );
    //CHECK_STATUS(XSetScreenSaver,1);
    return Val_unit;
}

CAMLprim value
ml_XActivateScreenSaver( value dpy )
{
    //GET_STATUS
    XActivateScreenSaver( Display_val(dpy) );
    //CHECK_STATUS(XActivateScreenSaver,1);
    return Val_unit;
}

CAMLprim value
ml_XResetScreenSaver( value dpy )
{
    //GET_STATUS
    XResetScreenSaver( Display_val(dpy) );
    //CHECK_STATUS(XResetScreenSaver,1);
    return Val_unit;
}

/* {{{ (Screen *) */

CAMLprim value
ml_XDefaultScreenOfDisplay( value dpy )
{
    Screen *scr = _xDefaultScreenOfDisplay(
        Display_val(dpy) );
    return Val_XScreen(scr);
}

CAMLprim value
ml_XScreenOfDisplay( value dpy, value screen_number )
{
    Screen *scr = _xScreenOfDisplay(
        Display_val(dpy),
        ScreenNB_val(screen_number) );
    return Val_XScreen(scr);
}

CAMLprim value
ml_XDefaultVisualOfScreen( value xscreen )
{
    Visual *visual = _xDefaultVisualOfScreen(
        XScreen_val(xscreen) );
    return Val_Visual(visual);
}

CAMLprim value
ml_XRootWindowOfScreen( value xscreen )
{
    return Val_Window( _xRootWindowOfScreen( XScreen_val(xscreen) ));
}

CAMLprim value
ml_XBlackPixelOfScreen( value xscreen )
{
    unsigned long px = _xBlackPixelOfScreen(
        XScreen_val(xscreen)
    );
    return Val_pixel_color(px);
}

CAMLprim value
ml_XWhitePixelOfScreen( value xscreen )
{
    unsigned long px = _xWhitePixelOfScreen(
        XScreen_val(xscreen)
    );
    return Val_pixel_color(px);
}

CAMLprim value
ml_XDefaultColormapOfScreen( value xscreen )
{
    Colormap cmap = _xDefaultColormapOfScreen(
        XScreen_val(xscreen)
    );
    return Val_Colormap(cmap);
}

CAMLprim value
ml_XDefaultDepthOfScreen( value xscreen )
{
    return Val_int( _xDefaultDepthOfScreen( XScreen_val(xscreen) ));
}

CAMLprim value
ml_XDefaultGCOfScreen( value xscreen )
{
    GC gc = _xDefaultGCOfScreen( XScreen_val(xscreen) );
    Display *dpy = _xDisplayOfScreen( XScreen_val(xscreen) );
    return Val_GC(gc, Val_Display(dpy));
}

CAMLprim value
ml_XDisplayOfScreen( value xscreen )
{
    Display *dpy = _xDisplayOfScreen(
        XScreen_val(xscreen)
    );
    return Val_Display(dpy);
}

CAMLprim value
ml_XWidthOfScreen( value xscreen )
{
    return Val_int( _xWidthOfScreen( XScreen_val(xscreen) ));
}

CAMLprim value
ml_XHeightOfScreen( value xscreen )
{
    return Val_int( _xHeightOfScreen( XScreen_val(xscreen) ));
}

CAMLprim value
ml_XScreenNumberOfScreen( value xscreen )
{
    return Val_screenNB( XScreenNumberOfScreen( XScreen_val(xscreen) ));
}

/* }}} */
/* {{{ ICCCM routines */

#if 0

Status XReconfigureWMWindow(
    Display*            /* display */,
    Window              /* w */,
    int                 /* screen_number */,
    unsigned int        /* mask */,
    XWindowChanges*     /* changes */
);  
    
Status XGetWMProtocols(
    Display*            /* display */,
    Window              /* w */,
    Atom**              /* protocols_return */,
    int*                /* count_return */
);  
Status XSetWMProtocols(
    Display*            /* display */,
    Window              /* w */,
    Atom*               /* protocols */,
    int                 /* count */
);
#endif

CAMLprim value
ml_XIconifyWindow( value dpy, value win, value screen_number )
{
    GET_STATUS XIconifyWindow(
        Display_val(dpy),
        Window_val(win),
        ScreenNB_val(screen_number)
    );
    CHECK_STATUS(XIconifyWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XWithdrawWindow( value dpy, value win, value screen_number )
{
    GET_STATUS XWithdrawWindow(
        Display_val(dpy),
        Window_val(win),
        ScreenNB_val(screen_number)
    );
    CHECK_STATUS(XWithdrawWindow,1);
    return Val_unit;
}

CAMLprim value
ml_XGetCommand( value dpy, value win )
{
    CAMLparam2(dpy, win);
    CAMLlocal1(cmds);
    char** argv;
    int argc = 0;
    Status st = XGetCommand(
        Display_val(dpy),
        Window_val(win),
        &argv,
        &argc
    );
    if (!st) {
        cmds = caml_alloc(0, 0);  // return an empty array instead of an exception
    } else {
        int i;
        cmds = caml_alloc(argc, 0);
        for (i=0; i<argc; ++i) {
            Store_field(cmds, i, caml_copy_string(argv[i]));
        }
        XFreeStringList(argv);
    }
    CAMLreturn(cmds);
}

#if 0

Status XGetWMColormapWindows(
    Display*            /* display */,
    Window              /* w */,
    Window**            /* windows_return */,
    int*                /* count_return */
);
Status XSetWMColormapWindows(
    Display*            /* display */,
    Window              /* w */,
    Window*             /* colormap_windows */,
    int                 /* count */
);
void XFreeStringList(
    char**              /* list */
);
int XSetTransientForHint(
    Display*            /* display */,
    Window              /* w */,
    Window              /* prop_window */
);

#endif

/* }}} */
/* {{{ Cursor */

/*
    XC_arrow,
    XC_top_left_arrow,
    XC_hand1,
    XC_pirate,
    XC_question_arrow,
    XC_exchange,
    XC_spraycan,
    XC_watch,
    XC_xterm,
    XC_crosshair,
    XC_sb_v_double_arrow,
    XC_sb_h_double_arrow,
    XC_top_side,
    XC_bottom_side,
    XC_left_side,
    XC_right_side,
    XC_top_left_corner,
    XC_top_right_corner,
    XC_bottom_right_corner,
    XC_bottom_left_corner,
*/

#include <X11/cursorfont.h>

static const unsigned int cursor_shape_table[] = {
    XC_X_cursor,
    XC_arrow,
    XC_based_arrow_down,
    XC_based_arrow_up,
    XC_boat,
    XC_bogosity,
    XC_bottom_left_corner,
    XC_bottom_right_corner,
    XC_bottom_side,
    XC_bottom_tee,
    XC_box_spiral,
    XC_center_ptr,
    XC_circle,
    XC_clock,
    XC_coffee_mug,
    XC_cross,
    XC_cross_reverse,
    XC_crosshair,
    XC_diamond_cross,
    XC_dot,
    XC_dotbox,
    XC_double_arrow,
    XC_draft_large,
    XC_draft_small,
    XC_draped_box,
    XC_exchange,
    XC_fleur,
    XC_gobbler,
    XC_gumby,
    XC_hand1,
    XC_hand2,
    XC_heart,
    XC_icon,
    XC_iron_cross,
    XC_left_ptr,
    XC_left_side,
    XC_left_tee,
    XC_leftbutton,
    XC_ll_angle,
    XC_lr_angle,
    XC_man,
    XC_middlebutton,
    XC_mouse,
    XC_pencil,
    XC_pirate,
    XC_plus,
    XC_question_arrow,
    XC_right_ptr,
    XC_right_side,
    XC_right_tee,
    XC_rightbutton,
    XC_rtl_logo,
    XC_sailboat,
    XC_sb_down_arrow,
    XC_sb_h_double_arrow,
    XC_sb_left_arrow,
    XC_sb_right_arrow,
    XC_sb_up_arrow,
    XC_sb_v_double_arrow,
    XC_shuttle,
    XC_sizing,
    XC_spider,
    XC_spraycan,
    XC_star,
    XC_target,
    XC_tcross,
    XC_top_left_arrow,
    XC_top_left_corner,
    XC_top_right_corner,
    XC_top_side,
    XC_top_tee,
    XC_trek,
    XC_ul_angle,
    XC_umbrella,
    XC_ur_angle,
    XC_watch,
    XC_xterm,
};

CAMLprim value
ml_XCreateFontCursor( value dpy, value shape )
{
    Cursor cur = XCreateFontCursor(
        Display_val(dpy),
        cursor_shape_table[ Long_val(shape) ]
    );
    return Val_Cursor(cur);
}

CAMLprim value
ml_XDefineCursor( value dpy, value win, value cur )
{
    //GET_STATUS
    XDefineCursor(
        Display_val(dpy),
        Window_val(win),
        Cursor_val(cur)
    );
    //CHECK_STATUS(XDefineCursor,1);
    return Val_unit;
}

CAMLprim value
ml_XRecolorCursor( value dpy, value cur, value foreground, value background )
{
    //GET_STATUS
    XRecolorCursor(
        Display_val(dpy),
        Cursor_val(cur),
        XColor_val(foreground),
        XColor_val(background)
    );
    //CHECK_STATUS(XRecolorCursor,1);
    return Val_unit;
}

/* }}} */
/* {{{ Font */

#define XFontStruct_val(v) ((XFontStruct *)(v))
#define Val_XFontStruct(fs) ((value)(fs))

CAMLprim value
ml_XSetFont( value dpy, value gc, value font )
{
    //GET_STATUS
    XSetFont(
        Display_val(dpy),
        GC_val(gc),
        Font_val(font)
    );
    //CHECK_STATUS(XSetFont,1);
    return Val_unit;
}

CAMLprim value
ml_XLoadQueryFont( value dpy, value name )
{
    XFontStruct *fs = XLoadQueryFont(
        Display_val(dpy),
        String_val(name)
    );
    return Val_XFontStruct(fs);
}

CAMLprim value
ml_XQueryFont( value dpy, value font )
{
    XFontStruct *fs = XQueryFont(
        Display_val(dpy),
        Font_val(font)
    );
    return Val_XFontStruct(fs);
}

CAMLprim value
ml_XQueryFontGC( value dpy, value gc )
{
    XFontStruct *fs = XQueryFont(
        Display_val(dpy),
        XGContextFromGC(GC_val(gc))
    );
    return Val_XFontStruct(fs);
}


#if 0
XFontStruct {
    XExtData    *ext_data;      /* hook for extension to hang data */
    Font        fid;            /* Font id for this font */
    unsigned    direction;      /* hint about direction the font is painted */
    unsigned    min_char_or_byte2;/* first character */
    unsigned    max_char_or_byte2;/* last character */
    unsigned    min_byte1;      /* first row that exists */
    unsigned    max_byte1;      /* last row that exists */
    Bool        all_chars_exist;/* flag if all characters have non-zero size*/
    unsigned    default_char;   /* char to print for undefined character */
    int         n_properties;   /* how many properties there are */
    XFontProp   *properties;    /* pointer to array of additional properties*/
    XCharStruct min_bounds;     /* minimum bounds over all existing char*/
    XCharStruct max_bounds;     /* maximum bounds over all existing char*/
    XCharStruct *per_char;      /* first_char to last_char information */
    int         ascent;         /* log. extent above baseline for spacing */
    int         descent;        /* log. descent below baseline for spacing */
}
#endif

#define FNTST_GET(field_c_type, field_name, Val_conv, ml_type) \
\
CAMLprim value \
ml_XFontStruct_get_##field_name( value vfs ) \
{ \
    return Val_conv((XFontStruct_val(vfs))->field_name); \
}

#define FNTST_GML(field_c_type, field_name, Val_conv, ml_type) \
external xFontStruct_##field_name: xFontStruct -> ml_type = quote(ml_XFontStruct_get_##field_name)

FNTST_GET(Font, fid,      Val_Font, font)
FNTST_GET(int,  ascent,   Val_int,  int)
FNTST_GET(int,  descent,  Val_int,  int)
FNTST_GET(Bool, all_chars_exist, Val_bool, bool)

CAMLprim value
ml_XFontStruct_get_height( value vfs )
{
    CAMLparam1( vfs );
    CAMLlocal1( height );
    XFontStruct * fs = XFontStruct_val(vfs);
    height = caml_alloc(2, 0);
    Store_field( height, 0, Val_int(fs->ascent) );
    Store_field( height, 1, Val_int(fs->descent) );
    CAMLreturn( height );
}

CAMLprim value
ml_XFontStruct_get_height2( value dpy, value gc )
{
    CAMLparam2( dpy, gc );
    CAMLlocal1( height );

    XFontStruct *fs = XQueryFont(
        Display_val(dpy),
        XGContextFromGC(GC_val(gc))
    );
    if (!fs)
        caml_failwith("XFontStruct");

    height = caml_alloc(2, 0);
    Store_field( height, 0, Val_int(fs->ascent) );
    Store_field( height, 1, Val_int(fs->descent) );
    CAMLreturn( height );
}

CAMLprim value
ml_XTextWidth( value vfs, value str )
{
    XFontStruct * fs = XFontStruct_val(vfs);
    //if (fs == NULL)
    //    caml_invalid_argument("xTextWidth");
    return Val_int( XTextWidth(
        fs,
        String_val(str),
        caml_string_length(str)
    ));
}

CAMLprim value
ml_xFontStruct_max_bounds( value vfs )
{
    CAMLparam1( vfs );
    CAMLlocal1( cst );
    XFontStruct * fs = XFontStruct_val(vfs);
    XCharStruct * b = &(fs->max_bounds);
    cst = caml_alloc(5, 0);
    Store_field( cst, 0, Val_int(b->lbearing) );
    Store_field( cst, 1, Val_int(b->rbearing) );
    Store_field( cst, 2, Val_int(b->width) );
    Store_field( cst, 3, Val_int(b->ascent) );
    Store_field( cst, 4, Val_int(b->descent) );
    CAMLreturn( cst );
}

CAMLprim value
ml_xFontStruct_min_bounds( value vfs )
{
    CAMLparam1( vfs );
    CAMLlocal1( cst );
    XFontStruct * fs = XFontStruct_val(vfs);
    XCharStruct * b = &(fs->min_bounds);
    cst = caml_alloc(5, 0);
    Store_field( cst, 0, Val_int(b->lbearing) );
    Store_field( cst, 1, Val_int(b->rbearing) );
    Store_field( cst, 2, Val_int(b->width) );
    Store_field( cst, 3, Val_int(b->ascent) );
    Store_field( cst, 4, Val_int(b->descent) );
    CAMLreturn( cst );
}

/*
int XTextWidth16(
    XFontStruct*      font_struct,
    _Xconst XChar2b*  string,
    int               count );
*/

/* }}} */

// vim: sw=4 sts=4 ts=4 et fdm=marker
