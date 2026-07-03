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
            .find_map(|p| load_spout_dll(p))?;

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

            let sender = Self {
                _lib: lib,
                obj,
                set_name,
                send_image,
            };

            let cname = CString::new(name).ok()?;
            (sender.set_name)(sender.obj, cname.as_ptr());
            Some(sender)
        }
    }

    pub fn send_rgba(&mut self, rgba: &[u8], width: u32, height: u32) -> bool {
        // Refuse a short buffer: passing it to native code would read past
        // the end of the slice. The QOI decoder always yields exactly w*h*4,
        // so this only triggers on a programming error.
        let needed = width as usize * height as usize * 4;
        if rgba.len() < needed {
            return false;
        }

        // No intermediate copy: Spout reads the pixels synchronously during
        // SendImage, so handing it the decode buffer directly is safe.
        unsafe {
            (self.send_image)(
                self.obj,
                rgba.as_ptr(),
                width,
                height,
                GL_RGBA,
                false,
            )
        }
    }

    #[allow(dead_code)]
    pub fn is_available() -> bool {
        Self::try_new("3DS2SPOUT-probe").is_some()
    }
}

fn load_spout_dll(path: &str) -> Option<libloading::Library> {
    if !Path::new(path).exists() {
        return None;
    }
    unsafe { libloading::Library::new(path).ok() }
}

impl Drop for SpoutSender {
    fn drop(&mut self) {
        // ReleaseSender is vtable index 2; optional cleanup
    }
}
