# Intro

DDX is a rugged, manually-switched multiband 10W digital transceiver.

# Power output

5W+ to 25W+ plus depending on the input voltage.

- 5W+ at 12v

- 10W+ at 20v (typically via a PD 3.0 USB battery bank)

- At higher voltages, more RF output power (25W+) is produced. Using an
  external fan is REQUIRED for such higher power levels.

  FT8 and WSPR have killed many Icom and other rigs before ;)

# Requirements

The following items are required for operating DDX:

- 12v battery (LFP / Li-ion packs are fine) or power supply

  However we recommend getting a PD 3.0 compatible USB battery bank. You can
  carry this USB battery bank in flight too and it can power your other non-HAM
  equipment.

  With a PD 3.0 compatible USB battery bank, we can do 10W+ easily in a
  portable fashion.

  Example product: `Amazon Basics Ultimate Power 20000mAh Power Bank (PD100W)`

- An antenna

  For field operations, we recommend getting / building an EFHW antenna.

  See https://github.com/kholia/HF-Balcony-Antenna-System for an example.

- Android phone or laptop

  The laptop can run the standard WSJT-X software.

  On Android, use our `FT8 Radio` app from Google Play Store.

- TX-BPF (eBay / AliExpress, *HIGHLY RECOMMENDED* for good performance and
  compliance, *REQUIRED* if you have radio interference problems)

  Cost: Around 45 USD

  This TX-BPF helps your other radios as well and is a one-time investment.

  ![TX-BPF 1](./Pictures/BPF-1-reduced.png)

  ![TX-BPF 2](./Pictures/BPF-28-1-reduced.png)

  If you have the budget, better TX-BPF filters are available from SurgeStop
  (Morgan Systems).

  NB: Please ONLY BUY DDX IF YOU HAVE A SUITABLE TX-BPF FILTER!

- [Optional] USD PD trigger-decoy module (when using PD 3.0 battery bank)

  ![USB PD decoy module](./screenshots/pd-decoy-module.jpg)

- [Optional] Common-mode choke (CMC)

  Can be useful depending on your operating conditions.

# Assembly steps

- Connect the fully asembled DDX board to a heat sink (heat sink can supplied
  as an option).

  ![Heat Sink](./screenshots/heat-sink-reduced.jpg)

- Put the DDX board into a suitable enclosure of your choice. This is optional
  for home usage.

- Done!

# Usage (with WSJT-X)

* Configure WSJT-X with your callsign and grid.

  ![WSJT-X 1](./screenshots/WSJTX-1.png)

* Select `MicNode` as the Soundcard Input.

  ![WSJT-X 2](./screenshots/WSJTX-2.png)

* Ensure the following default settings are present.

  ![WSJT-X 3](./screenshots/WSJTX-Settings-3.png)

* Launch the [wsjtx_helper/transceiver_server.exe](https://github.com/kholia/DDX/raw/refs/heads/master/wsjtx_helper/transceiver_server.exe) program.

* That's it!

# Usage (with `FT8 Radio` app on Android)

* Install `FT8 Radio` app from Google Play Store.

* Launch the app and go to settings (`gear icon`).

  Configure settings and choose `DDX` as the radio.

  NB: Restart the app forcefully.

  ![FT8 Radio](./screenshots/app-reduced.jpg)

* Connect DDX to the phone using a Micro-USB cable and a reliable OTG adapter.

* Done!

PS: See https://github.com/kholia/DigitalRadioReceiverSupport/tree/master/NewApp for more help.

# Tips

For best results, we recommend syncing time on your laptop / PC using
https://www.timesynctool.com/ software.

# Features

- Comes as a fully assembled and fully tested-on-air board

- Single USB cable for handling both Audio + CAT control

  DDX has a built-in 'sound card'

- True 10W+ output on all HF bands (with 20v from a PD 3.0 USB battery bank)

- Rock-solid PA which handles open, short, and bad SWR conditions (1:10 SWR
  tested)

- Rock-steady VFO which does NOT drift even under exposed conditions. Yes - it
  receives WSPR at 28 MHz just fine.

- Reliable T/R switching based on a DPDT relay

- No toroids to wind assuming an external LPF bank is in use ;-)

- The final MOSFET remains as cool as a cucumber (Class-D operation)

- DDX has a bit-perfect, deterministic TX output

- It works beautifully with the 'FT8 Radio' app
  (https://github.com/kholia/DigitalRadioReceiverSupport/). The 'FT8 Radio' app
  handles FT8 decoding well in real time (Native MCU decoders offer significantly
  reduced performance at the moment). The app can decode 35++ stations at a time,
  and even large screens can fill up very rapidly - such is the receiver's
  performance (with a BPF)!

- No calibration is (ever) required. Just pick up DDX and it is ready to go on
  air. Comes with 26 MHz 0.5ppm TCXO as standard.

- Low cost (cost-effective)

- Stress tested by WSJT-Z for multiple days

- DDX routinely receives FT8 traffic from 80 to 100+ countries using a simple
  5m long EFHW antenna with a BPF attached

# Firmware Upgrade

- NB: Remove all sources of external DC power

- Unplug the Pico MCU board from the computer

- Keep the white button on the Pico MCU board pressed, and connect Pico to the
  computer via a USB cable

- Upload the supplied `.uf2` firmware file to the new drive that opens up on
  your computer

- Done!

# Render

![DDX-Commericial-1](./screenshots/DDX-Commercial-3.png)

# Resources

- [The Global Communications Transceiver](https://www.youtube.com/watch?v=b82O-rVQVpk) - Older DDX version in action

- [FT8 Radio App demo](https://www.youtube.com/watch?v=OX8Knu0yh6M)

- [DDX contest writeup](https://www.electronicwings.com/users/DhiruKholia/projects/4382/global-communications-transceiver)
