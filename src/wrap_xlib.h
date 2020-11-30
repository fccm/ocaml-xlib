#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


//#define Display_val(v) ((Display *)(v))
//#define Val_Display(dpy) ((value)(dpy))


#define Display_val(v) ((Display *)(Field((v),0)))

static inline value
Val_Display(Display * dpy)
{
// Store information to know if the display is currently opened
    CAMLparam0();
    CAMLlocal1( ml_dpy );
    ml_dpy = caml_alloc(2, 0);
    Store_field( ml_dpy, 0, ((value)(dpy)) );
    Store_field( ml_dpy, 1, Val_true );
    CAMLreturn( ml_dpy );
}

#define display_record_closed(dpy) Store_field((dpy), 1, Val_false)
#define display_is_open(dpy) (Field((dpy),1) == Val_true)


static inline value
Val_some(value v)
{   
    CAMLparam1(v);
    CAMLlocal1(some);
    some = caml_alloc(1, 0);
    Store_field(some, 0, v);
    CAMLreturn(some);
}

#define Val_none Val_int(0)

#define Val_char Val_int
#define Char_val Int_val


#define Val_screenNB Val_int
#define ScreenNB_val Int_val


/* The scruture Screen is here renamed with an additionnal X
   as a way of disambiguation with screen numbers (int) and 
   its conversion macro ScreenNB_val() */
value Val_XScreen(Screen *s)
{
    return caml_copy_nativeint((intnat) s);
}

Screen * XScreen_val(value v)
{
    return (Screen *) Nativeint_val(v);
}


#define XID_val(type,v) ((type) (v))
#define Val_XID(v) ((value) (v))

#define Window_val(v) (XID_val(Window,(v)))
#define Val_Window(xid) (Val_XID((xid)))

#define Pixmap_val(v) (XID_val(Pixmap,(v)))
#define Val_Pixmap(xid) (Val_XID((xid)))

#define Font_val(v) (XID_val(Font,(v)))
#define Val_Font(xid) (Val_XID((xid)))


/* pointers */

//#define XVisualInfo_val(v) ((XVisualInfo *)(v))
//#define Val_XVisualInfo(vi) ((value)(vi))

#define Val_Visual(vis) ((value)(vis))
#define Visual_val(v) ((Visual *)(v))


#define VisualID_val(v) ((VisualID)(v))
#define Val_VisualID(v) ((value)(v))


/* unsigned integers */

#define assert_wrong (assert(0), Long_val(0))

#define UInt_noassert_val(x) ((unsigned int) Long_val(x))
#define UInt_assert_val(x) (Long_val(x) < 0 ? assert_wrong : UInt_noassert_val(x) )

#define ULong_noassert_val(x) ((unsigned long) Long_val(x))
#define ULong_assert_val(x) (Long_val(x) < 0 ? assert_wrong : ULong_noassert_val(x) )


static inline unsigned long _ULong_warn_val(value x, const char *name) {
    if (Long_val(x) < 0) {
        fprintf(stderr, "negative: %s\n", name);
       	fflush(stderr);
    }
    return Long_val(x);
}
#define ULong_warn_val(x) _ULong_warn_val(x, #x)


#define ASSERT_SIGN_FOR_CONV 0
#define WARN_NEGATIVE 0

#if ASSERT_SIGN_FOR_CONV
  #define ULong_selected_val ULong_assert_val
  #define UInt_selected_val UInt_assert_val
#else
#if WARN_NEGATIVE
  #define ULong_selected_val(x) ((unsigned long)ULong_warn_val(x))
  #define UInt_selected_val(x) ((unsigned int)ULong_warn_val(x))
#else
  #define ULong_selected_val ULong_noassert_val
  #define UInt_selected_val UInt_noassert_val
#endif
#endif



#define UInt_val UInt_selected_val
#define Val_uint Val_int

#define ULong_val ULong_selected_val
#define Val_ulong Val_long



/* caml allocs */

/* Default custom_operations */
#define custom_ops(s) \
static struct custom_operations s##_custom_ops = { \
    identifier: #s, \
    finalize:    custom_finalize_default, \
    compare:     custom_compare_default, \
    hash:        custom_hash_default, \
    serialize:   custom_serialize_default, \
    deserialize: custom_deserialize_default \
}

#define copy_obj(type, obj, ml_return) \
  do{ ml_return = caml_alloc_custom( &type##_custom_ops, sizeof(type), 0, 1); \
      memcpy( Data_custom_val(ml_return), &obj, sizeof(type) ); }while(0)

#define alloc_obj(type, ml_return) \
    ml_return = caml_alloc_custom( &type##_custom_ops, sizeof(type), 0, 1);

#define custom_ops_n(s)  custom_ops(s##_n)

#define alloc_n_obj(type, ml_return, n) \
    ml_return = caml_alloc_custom( &type##_n_custom_ops, n * sizeof(type), 0, 1);


#define copy_XEvent(e,v) copy_obj(XEvent,(e),(v))

#define alloc_XEvent(v) alloc_obj(XEvent,(v))
#define alloc_XColor(v) alloc_obj(XColor,(v))
#define alloc_XGCValues(v) alloc_obj(XGCValues,(v))
#define alloc_XSetWindowAttributes(v) alloc_obj(XSetWindowAttributes,(v))
#define alloc_XWindowAttributes(v) alloc_obj(XWindowAttributes,(v))
#define alloc_XSizeHints(v) alloc_obj(XSizeHints,(v))
#define alloc_XVisualInfo(v) alloc_obj(XVisualInfo,(v))
#define alloc_XChar2b(v) alloc_obj(XChar2b,(v))
#define alloc_n_XChar2b(v, n) alloc_n_obj(XChar2b,(v),(n))

#define XEvent_val(v) ((XEvent *)Data_custom_val(v))
#define XColor_val(v) ((XColor *)Data_custom_val(v))
#define XGCValues_val(v) ((XGCValues *)Data_custom_val(v))
#define XSetWindowAttributes_val(v) ((XSetWindowAttributes *)Data_custom_val(v))
#define XWindowAttributes_val(v) ((XWindowAttributes *)Data_custom_val(v))
#define XSizeHints_val(v) ((XSizeHints *)Data_custom_val(v))
#define XVisualInfo_val(v) ((XVisualInfo *)Data_custom_val(v))
#define XChar2b_val(v) ((XChar2b *)Data_custom_val(v))
#define XChar2b_ptr_val XChar2b_val

#define XChar2b_string_val(v) XChar2b_ptr_val(Field(v,0))
#define XChar2b_string_length(v) Long_val(Field(v,1))


/* debug functions */

#define do_warn0(msg) \
    do{ fprintf(stderr,msg); fflush(stderr); }while(0)

#define do_warn1(msg,p1) \
    do{ fprintf(stderr,msg,p1); fflush(stderr); }while(0)

#define do_warn2(msg,p1,p2) \
    do{ fprintf(stderr,msg,p1,p2); fflush(stderr); }while(0)

#define do_warn3(msg,p1,p2,p3) \
    do{ fprintf(stderr,msg,p1,p2,p3); fflush(stderr); }while(0)


