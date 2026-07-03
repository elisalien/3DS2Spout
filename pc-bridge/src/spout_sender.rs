use std::ffi::CString;
use std::path::Path;

const GL_RGBA: u32 = 0x1908;

type GetSpoutFn = unsafe extern "C" fn() -> *mut std::ffi::c_void;

type SetSenderNameFn =
    unsafe extern "C" fn(this: *mut std::ffi::c_void, name: *const i8);
type SendImageFn = unsafe extern "C" fn(
    this: *mut std::ffi::c_void,
    pixels: *const u8,
    width: u32,
    height: u32,
    gl_format: u32,
    invert: bool,
) -> bool;

pub struct SpoutSender {
    _lib: libloading::Library,
    obj: *mut std::ffi::c_void,
    set_name: SetSenderNameFn,
    send_image: SendImageFn,
    rgba_buf: Vec<u8>,
    width: u32,
    height: u32,
}

unsafe impl Send for SpoutSender {}

impl SpoutSender {
    pub fn try_new(name: &str) -> Option<Self> {
        let dll_paths = [
            "SpoutLibrary.dll",
            "vendor/SpoutLibrary.dll",
            "../vendor/SpoutLibrary.dll",
            "pc-bridge/vendor/SpoutLibrary.dll",
        ];

        let lib = dll_paths
            .iter()
            .find_map(|p| load_spout_dll(p).ok())?;

        unsafe {
            let get_spout: libloading::Symbol<GetSpoutFn> =
                lib.get(b"GetSpout").ok()?;
            let obj = get_spout();
            if obj.is_null() {
                return None;
            }

            let vtable = *(obj as *const *const usize);
            let set_name: SetSenderNameFn = std::mem::transmute(*vtable.add(0));
            let send_image: SendImageFn = std::mem::transmute(*vtable.add(5));

            let mut sender = Self {
                _lib: lib,
                obj,
                set_name,
                send_image,
                rgba_buf: Vec::new(),
                width: 0,
                height: 0,
            };

            let cname = CString::new(name).ok()?;
            (sender.set_name)(sender.obj, cname.as_ptr());
            Some(sender)
        }
    }

    pub fn send_rgba(&mut self, rgba: &[u8], width: u32, height: u32) -> bool {
        let needed = (width * height * 4) as usize;
        if self.rgba_buf.len() != needed {
            self.rgba_buf.resize(needed, 0);
        }
        self.rgba_buf[..needed.min(rgba.len())].copy_from_slice(&rgba[..needed.min(rgba.len())]);
        self.width = width;
        self.height = height;

        unsafe {
            (self.send_image)(
                self.obj,
                self.rgba_buf.as_ptr(),
                width,
                height,
                GL_RGBA,
                false,
            )
        }
    }

    pub fn is_available() -> bool {
        Self::try_new("3DS2SPOUT-probe").is_some()
    }
}

fn load_spout_dll(path: &str) -> Result<libloading::Library, libloading::Error> {
    if Path::new(path).exists() {
        unsafe { libloading::Library::new(path) }
    } else {
        unsafe { libloading::Library::new(path) }
    }
}

impl Drop for SpoutSender {
    fn drop(&mut self) {
        // ReleaseSender is vtable index 2; optional cleanup
    }
}
