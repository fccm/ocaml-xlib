/* OCaml bindings for the Xt library (as part of OCaml-Xlib).
   Copyright (C) 2008 by Florent Monnier
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#define CAML_NAME_SPACE 1

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/signals.h>

#include "wrap_xlib.h"


// Widget is a pointer to a structure
#define Val_Widget(wid) ((value)(wid))
#define Widget_val(v) ((Widget)(v))

// XtAppContext is a pointer to a structure
//#define Val_XtAppContext(app) ((value)(app))
//#define XtAppContext_val(v) ((XtAppContext)(v))

custom_ops(XtAppContext);
#define alloc_XtAppContext(v) alloc_obj(XtAppContext,(v))
#define XtAppContext_val(v) ((XtAppContext *)Data_custom_val(v))

static WidgetClass get_shellWidgetClass( value i )
{
    switch (Long_val(i)) {
        case 0: return shellWidgetClass;
        case 1: return overrideShellWidgetClass;
        case 2: return wmShellWidgetClass;
        case 3: return transientShellWidgetClass;
        case 4: return topLevelShellWidgetClass;
        case 5: return applicationShellWidgetClass;
        case 6: return sessionShellWidgetClass;
    }
    return shellWidgetClass; // just to avoid a warning
}

CAMLprim value
ml_XtOpenApplication( value application_class, value ml_widget_class, value ml_app_resources )
{
    CAMLparam3(application_class, ml_widget_class, ml_app_resources);
    CAMLlocal2(ret, app_context);
    alloc_XtAppContext(app_context);

    WidgetClass widget_class = get_shellWidgetClass(ml_widget_class);
    char *p_argv[] = { };
    int p_argc = 0;

    /*
    String app_resources[] = {
        "*command.Label: Write text to stdout",
        "*clear_command.Label: Clear",
        "*quit_command.Label: Quit",
        "*window.Title: Hello, world in Xt/Athena",
        "*window.Geometry: 300x200+10+10",
        "*ascii.Width: 280",
        "*ascii.Height: 150",
        NULL };
    */
    String * app_resources;
    app_resources = calloc(Wosize_val(ml_app_resources) + 1, sizeof(String *));
    int i;
    for (i=0; i<Wosize_val(ml_app_resources); ++i)
        app_resources[i] = String_val(Field(ml_app_resources, i));
    app_resources[i] = NULL;

    Widget window = XtOpenApplication(
        XtAppContext_val(app_context),
        String_val(application_class),
        NULL,    // XrmOptionDescList  options,
        0,       // Cardinal           num_options,
        &p_argc, // int*               argc_in_out,
        p_argv,  // String*            argv_in_out,
            //NULL,    // String*            app_resources,
            app_resources,
        widget_class,
        NULL,    // ArgList            args,
        0        // Cardinal           num_args
        );

    ret = caml_alloc(2, 0);
    Store_field(ret, 0, app_context );
    Store_field(ret, 1, Val_Widget(window) );
    CAMLreturn(ret);
}

CAMLprim value
ml_XtAppMainLoop( value app_context )
{
    XtAppMainLoop(
        *(XtAppContext_val(app_context)) );
    return Val_unit;
}
    
CAMLprim value
ml_XtDestroyApplicationContext( value app_context )
{
    XtDestroyApplicationContext(
        *(XtAppContext_val(app_context)) );
    return Val_unit;
}

CAMLprim value
ml_XtRealizeWidget( value widget )
{
    XtRealizeWidget( Widget_val(widget) );
    return Val_unit;
}

CAMLprim value
ml_XtManageChild( value child )
{
    XtManageChild( Widget_val(child) );
    return Val_unit;
}

CAMLprim value
ml_XtUnmanageChild( value child )
{
    XtUnmanageChild( Widget_val(child) );
    return Val_unit;
}

// {{{ Information routines 

CAMLprim value
ml_XtDisplay( value widget )
{
    Display *dpy = XtDisplay( Widget_val(widget) );
    return Val_Display(dpy);
}

CAMLprim value
ml_XtDisplayOfObject( value object )
{
    Display *dpy = XtDisplayOfObject( Widget_val(object) );
    return Val_Display(dpy);
}

CAMLprim value
ml_XtScreen( value widget )
{
    Screen *xscreen = XtScreen( Widget_val(widget) );
    return Val_XScreen(xscreen);
}

/*
Screen *XtScreenOfObject(
    Widget  object 
);
*/

CAMLprim value
ml_XtWindow( value widget )
{
    Window win = XtWindow( Widget_val(widget) );
    return Val_Window(win);
}

/*
Window XtWindowOfObject(
    Widget    object 
);

String XtName(
    Widget     object 
);

WidgetClass XtSuperclass(
    Widget     object 
);

WidgetClass XtClass(
    Widget        object 
);
*/

CAMLprim value
ml_XtParent( value widget )
{
    Widget parent = XtParent( Widget_val(widget) );
    return Val_Widget(parent);
}

// }}}

CAMLprim value
ml_XtMapWidget( value w )
{
    XtMapWidget( Widget_val(w) );
    return Val_unit;
}

CAMLprim value
ml_XtUnmapWidget( value w )
{
    XtUnmapWidget( Widget_val(w) );
    return Val_unit;
}

static const _Xconst _XtString  callback_name_table[] = {
    XtNcallback,
    XtNdestroyCallback,
    XtNpopupCallback,
    XtNpopdownCallback,
    XtNunrealizeCallback,
    XtCCallback,
    XtRCallback,
    XtHaddCallback,
    XtHaddCallbacks,
    XtHremoveCallback,
    XtHremoveCallbacks,
    XtHremoveAllCallbacks,
};

static value caml_xt_cb = 0;

CAMLprim value
init_xtAddCallback(value v)
{ 
  caml_xt_cb = v;
  caml_register_global_root(&caml_xt_cb);
  return Val_unit;
}

void ml_xt_callback( Widget w, XtPointer cb_index, XtPointer cb_data )
{
    caml_leave_blocking_section();
    caml_callback2( caml_xt_cb, (value) cb_index, (value) cb_data );
    //caml_callback( caml_xt_cb, (value) cb_index );
    caml_enter_blocking_section();
}

CAMLprim value
ml_XtAddCallback( value widget, value cb_name, value cb_index )
{
    _Xconst _XtString  callback_name = callback_name_table[Long_val(cb_name)];
    XtAddCallback(
        Widget_val(widget),
        callback_name,
        ml_xt_callback,
        (void *) cb_index );
    return Val_unit;
}


#include <X11/Xaw/AsciiSink.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>

WidgetClass get_WidgetClass(value i)
{
    switch (Long_val(i)) {
        case 0: return asciiSinkObjectClass;
        case 1: return asciiSrcObjectClass;
// AsciiText.h:typedef struct _AsciiTextClassRec        *AsciiTextWidgetClass;
        case 2: return asciiTextWidgetClass;
    //  case 3: return asciiStringWidgetClass;
    //  case 3: return asciiDiskWidgetClass;
        case 3: return boxWidgetClass;
// Box.h:typedef struct _BoxClassRec *BoxWidgetClass;
        case 4: return commandWidgetClass;
// Command.h:typedef struct _CommandClassRec   *CommandWidgetClass;
// Dialog.h:typedef struct _DialogClassRec        *DialogWidgetClass;
// Dialog.h:extern WidgetClass dialogWidgetClass;
// Form.h:typedef struct _FormClassRec        *FormWidgetClass;
        case 5: return formWidgetClass;
        case 6: return gripWidgetClass;
// Grip.h:typedef struct _GripClassRec *GripWidgetClass;
        case 7: return labelWidgetClass;
// Label.h:typedef struct _LabelClassRec *LabelWidgetClass;
    //  case 8: return listWidgetClass;
// List.h:typedef struct _ListClassRec *ListWidgetClass;
        case 8: return menuButtonWidgetClass;
// MenuButton.h:typedef struct _MenuButtonClassRec   *MenuButtonWidgetClass;
// MultiSink.h:extern WidgetClass multiSinkObjectClass;
// MultiSrc.h:extern WidgetClass multiSrcObjectClass;
// Paned.h:extern WidgetClass panedWidgetClass;
// Paned.h:typedef struct _PanedClassRec        *PanedWidgetClass;
// Panner.h:extern WidgetClass pannerWidgetClass;
// Panner.h:typedef struct _PannerClassRec *PannerWidgetClass;
// Porthole.h:extern WidgetClass portholeWidgetClass;
// Porthole.h:typedef struct _PortholeClassRec *PortholeWidgetClass;
// Print.h:extern WidgetClass xawPrintShellWidgetClass;
// PrintSP.h:extern WidgetClass xawPrintShellWidgetClass;
// Repeater.h:extern WidgetClass repeaterWidgetClass;
// Repeater.h:typedef struct _RepeaterClassRec *RepeaterWidgetClass;
// Scrollbar.h:typedef struct _ScrollbarClassRec *ScrollbarWidgetClass;
// Scrollbar.h:extern WidgetClass scrollbarWidgetClass;
// Simple.h:typedef struct _SimpleClassRec        *SimpleWidgetClass;
// Simple.h:extern WidgetClass simpleWidgetClass;
// SimpleMenP.h:    WidgetClass        label_class;        /* Widget Class of the menu label object */
// SimpleMenu.h:typedef struct _SimpleMenuClassRec*        SimpleMenuWidgetClass;
// SimpleMenu.h:extern WidgetClass simpleMenuWidgetClass;
// SmeBSB.h:extern WidgetClass smeBSBObjectClass;
// Sme.h:extern WidgetClass smeObjectClass;
// SmeLine.h:extern WidgetClass smeLineObjectClass;
// StripChart.h:typedef struct _StripChartClassRec *StripChartWidgetClass;
// StripChart.h:extern WidgetClass stripChartWidgetClass;
// Template.c:    (WidgetClass)Superclass,                /* superclass */
// Template.c:WidgetClass templateWidgetClass = (WidgetClass)&templateClassRec;
// Template.h:typedef struct _TemplateClassRec *TemplateWidgetClass;
// Template.h:extern WidgetClass templateWidgetClass;
// Text.h: Class:         textWidgetClass
// Text.h:extern WidgetClass textWidgetClass;
// Text.h:typedef struct _TextClassRec *TextWidgetClass;
// TextSink.h:extern WidgetClass textSinkObjectClass;
// TextSrc.h:extern WidgetClass textSrcObjectClass;
// Tip.h:typedef struct _TipClassRec *TipWidgetClass;
// Tip.h:extern WidgetClass tipWidgetClass;
// Toggle.h:extern WidgetClass               toggleWidgetClass;
// Toggle.h:typedef struct _ToggleClassRec   *ToggleWidgetClass;
// Tree.h:extern WidgetClass treeWidgetClass;
// Tree.h:typedef struct _TreeClassRec *TreeWidgetClass;
// Viewport.h:extern WidgetClass viewportWidgetClass;
// Viewport.h:typedef struct _ViewportClassRec *ViewportWidgetClass;
// ViewportP.h: * Private declarations for ViewportWidgetClass
// XawInit.h:    WidgetClass   widget_class,
    }
    return labelWidgetClass; // just to avoid a warning
}

CAMLprim value
ml_XtCreateManagedWidget( value name, value ml_widget_class, value parent )
{
    WidgetClass widget_class = get_WidgetClass(ml_widget_class);
    Widget wid = XtCreateManagedWidget(
        String_val(name),
        widget_class,
        Widget_val(parent),
        NULL,  // ArgList  args,
        0      // Cardinal num_args
    );
    return Val_Widget(wid);
}


// vim: sw=4 sts=4 ts=4 et fdm=marker
