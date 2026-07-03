use rosc::{OscMessage, OscPacket, OscType};
use std::net::UdpSocket;

#[derive(Debug, Clone, serde::Deserialize)]
pub struct OscMapping {
    pub param_id: u8,
    pub osc_address: String,
    #[serde(default = "default_osc_type")]
    pub osc_type: String,
}

fn default_osc_type() -> String {
    "float".to_string()
}

#[derive(Debug, Clone, serde::Deserialize)]
pub struct BridgeConfig {
    #[serde(default = "default_listen")]
    pub listen_port: u16,
    #[serde(default = "default_spout")]
    pub spout_name: String,
    #[serde(default = "default_osc_host")]
    pub osc_host: String,
    #[serde(default = "default_osc_port")]
    pub osc_port: u16,
    #[serde(default)]
    pub mappings: Vec<OscMapping>,
}

fn default_listen() -> u16 {
    5000
}
fn default_spout() -> String {
    "3DS2SPOUT".to_string()
}
fn default_osc_host() -> String {
    "127.0.0.1".to_string()
}
fn default_osc_port() -> u16 {
    7000
}

pub struct OscRelay {
    socket: UdpSocket,
    target: String,
    mappings: Vec<OscMapping>,
}

impl OscRelay {
    pub fn new(host: &str, port: u16, mappings: Vec<OscMapping>) -> std::io::Result<Self> {
        let socket = UdpSocket::bind("0.0.0.0:0")?;
        Ok(Self {
            socket,
            target: format!("{host}:{port}"),
            mappings,
        })
    }

    pub fn relay_param(&self, param_id: u8, value: f32) {
        let Some(mapping) = self.mappings.iter().find(|m| m.param_id == param_id) else {
            return;
        };

        let osc_value = match mapping.osc_type.as_str() {
            "int" => OscType::Int(value.round() as i32),
            _ => OscType::Float(value),
        };

        let msg = OscMessage {
            addr: mapping.osc_address.clone(),
            args: vec![osc_value],
        };

        if let Ok(buf) = rosc::encoder::encode(&OscPacket::Message(msg)) {
            let _ = self.socket.send_to(&buf, &self.target);
        }
    }
}

pub fn load_config(path: &str) -> BridgeConfig {
    let candidates = [
        path.to_string(),
        format!("../resolume/{path}"),
        format!("resolume/{path}"),
        "../resolume/osc-map.toml".to_string(),
        "resolume/osc-map.toml".to_string(),
    ];

    for candidate in &candidates {
        if let Ok(text) = std::fs::read_to_string(candidate) {
            if let Ok(cfg) = toml::from_str(&text) {
                if candidate != path {
                    println!("Loaded config from {candidate}");
                }
                return cfg;
            }
        }
    }

    eprintln!("Warning: no config found, using defaults");
    BridgeConfig {
        listen_port: default_listen(),
        spout_name: default_spout(),
        osc_host: default_osc_host(),
        osc_port: default_osc_port(),
        mappings: vec![],
    }
}
