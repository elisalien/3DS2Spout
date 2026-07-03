mod osc_relay;
mod protocol;
mod qoi_decoder;
mod spout_sender;
mod udp_receiver;

use clap::Parser;
use osc_relay::{load_config, OscRelay};
use spout_sender::SpoutSender;
use udp_receiver::UdpReceiver;

#[derive(Parser, Debug)]
#[command(name = "pc-bridge", about = "3DS camera stream receiver for Resolume")]
struct Args {
    #[arg(short, long, default_value = "config.toml")]
    config: String,
}

fn lan_ipv4() -> Option<String> {
    let socket = std::net::UdpSocket::bind("0.0.0.0:0").ok()?;
    socket.connect("8.8.8.8:80").ok()?;
    Some(socket.local_addr().ok()?.ip().to_string())
}

fn print_pc_ip_banner(port: u16) {
    println!();
    println!("============================================");
    if let Some(ip) = lan_ipv4() {
        println!("  IP PC pour la 3DS : {ip}");
        println!("  Sur la 3DS : bouton IP PC -> saisir {ip}");
    } else {
        println!("  IP PC : (detectez avec ipconfig)");
        println!("  Sur la 3DS : bouton IP PC -> saisie tactile");
    }
    println!("  Port UDP : {port}");
    println!("============================================");
    println!();
}

fn main() {
    let args = Args::parse();
    let config = load_config(&args.config);

    println!("3DS2SPOUT — PC Bridge");
    println!("Listening UDP :{}", config.listen_port);
    print_pc_ip_banner(config.listen_port);
    println!("OSC relay → {}:{}", config.osc_host, config.osc_port);

    let mut spout = SpoutSender::try_new(&config.spout_name);
    match &spout {
        Some(_) => println!("Spout sender: {}", config.spout_name),
        None => eprintln!(
            "Warning: SpoutLibrary.dll not found — video decode only.\n\
             Copy SpoutLibrary.dll to vendor/ from https://github.com/leadedge/Spout2/releases"
        ),
    }

    let osc = OscRelay::new(&config.osc_host, config.osc_port, config.mappings.clone())
        .expect("Failed to create OSC socket");

    let mut receiver = UdpReceiver::bind(config.listen_port).expect("Failed to bind UDP port");

    loop {
        if let Err(e) = receiver.poll(&mut spout, &osc) {
            eprintln!("Receive error: {e}");
        }
    }
}
