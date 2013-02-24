
type xtAppContext
type 'a widget
type wobject

type shell_widget_class =
  | ShellWidgetClass
  | OverrideShellWidgetClass
  | WmShellWidgetClass
  | TransientShellWidgetClass
  | TopLevelShellWidgetClass
  | ApplicationShellWidgetClass
  | SessionShellWidgetClass

external xtOpenApplication: application_class:string -> shell_widget_class -> string array ->  xtAppContext * 'a widget = "ml_XtOpenApplication"
external xtAppMainLoop: app:xtAppContext -> unit = "ml_XtAppMainLoop"
external xtDestroyApplicationContext: app:xtAppContext -> unit = "ml_XtDestroyApplicationContext"

external xtRealizeWidget: widget:'a widget -> unit = "ml_XtRealizeWidget"
external xtManageChild: child:'a widget -> unit = "ml_XtManageChild"
external xtUnmanageChild: child:'a widget -> unit = "ml_XtUnmanageChild"

external xtDisplay: widget:'a widget -> Xlib.display = "ml_XtDisplay"
external xtDisplayOfObject: wobject:wobject widget -> Xlib.display = "ml_XtDisplayOfObject"
external xtScreen: widget:'a widget -> Xlib.xScreen = "ml_XtScreen"
external xtWindow: widget:'a widget -> Xlib.window = "ml_XtWindow"
external xtParent: widget:'a widget -> 'a widget = "ml_XtParent"

external xtMapWidget: widget:'a widget -> unit = "ml_XtMapWidget"
external xtUnmapWidget: widget:'a widget -> unit = "ml_XtUnmapWidget"

type widget_class =
  | AsciiSinkObjectClass
  | AsciiSrcObjectClass
  | AsciiTextWidgetClass
  | BoxWidgetClass
  | CommandWidgetClass
  | FormWidgetClass
  | GripWidgetClass
  | LabelWidgetClass
  | MenuButtonWidgetClass

external xtCreateManagedWidget: name:string -> widget_class -> parent:'a widget -> 'a widget = "ml_XtCreateManagedWidget"

type callback_name =
  | XtNcallback
  | XtNdestroyCallback
  | XtNpopupCallback
  | XtNpopdownCallback
  | XtNunrealizeCallback
  | XtCCallback
  | XtRCallback
  | XtHaddCallback
  | XtHaddCallbacks
  | XtHremoveCallback
  | XtHremoveCallbacks
  | XtHremoveAllCallbacks

external xtAddCallback: widget:'a widget -> name:callback_name -> i:int -> unit = "ml_XtAddCallback"

external init_xtAddCallback: (int -> unit -> unit) -> unit = "init_xtAddCallback"

let cb_tbl = Hashtbl.create 101

let real_callback i v =
  (Hashtbl.find cb_tbl i) v

let _ = 
  init_xtAddCallback real_callback

let cb_count = ref 0 

let xtAddCallback ~widget ~name ~cb:(cb:(data:'a -> value:'b -> unit)) ~data =
  let i = !cb_count in
  incr cb_count;
  Hashtbl.add cb_tbl i (fun value -> 
    (* Hashtbl.remove cb_tbl i; *)
    cb ~data ~value);
  xtAddCallback ~widget ~name ~i


