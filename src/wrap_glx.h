
// pointers to structures:

#define GLXContext_val(v) ((GLXContext)(v))
#define Val_GLXContext(c) ((value)(c))

#define GLXFBConfig_val(v) ((GLXFBConfig)(v))
#define Val_GLXFBConfig(c) ((value)(c))

// XID's

#define GLXPixmap_val(v) (XID_val(GLXPixmap,(v)))
#define Val_GLXPixmap(xid) (Val_XID((xid)))

#define GLXDrawable_val(v) (XID_val(GLXDrawable,(v)))
#define Val_GLXDrawable(xid) (Val_XID((xid)))

#define GLXFBConfigID_val(v) (XID_val(GLXFBConfigID,(v)))
#define Val_GLXFBConfigID(xid) (Val_XID((xid)))

// typedef XID GLXContextID;

#define GLXWindow_val(v) (XID_val(GLXWindow,(v)))
#define Val_GLXWindow(xid) (Val_XID((xid)))

#define GLXPbuffer_val(v) (XID_val(GLXPbuffer,(v)))
#define Val_GLXPbuffer(xid) (Val_XID((xid)))

