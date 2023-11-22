Build Process:

```
$ pwd
pico-sdk

$ git checkout develop; git checkout -f
```

[Optional] Apply the following patch:

```
$ git diff
diff --git a/src/boards/include/boards/waveshare_rp2040_zero.h b/src/boards/include/boards/waveshare_rp2040_zero.h
index 281b4dc..858f930 100644
--- a/src/boards/include/boards/waveshare_rp2040_zero.h
+++ b/src/boards/include/boards/waveshare_rp2040_zero.h
@@ -16,6 +16,10 @@
 // For board detection
 #define WAVESHARE_RP2040_ZERO

+#ifndef PICO_XOSC_STARTUP_DELAY_MULTIPLIER
+#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64
+#endif
+
 // --- UART ---
 #ifndef PICO_DEFAULT_UART
 #define PICO_DEFAULT_UART 0
```

```
export PICO_SDK_PATH=$HOME/repos/pico-sdk

export PICO_BOARD=waveshare_rp2040_zero  # note

cmake .

cmake -DCMAKE_BUILD_TYPE=Debug .  # for debugging

make -j8
```

```
$ pacmd list-sources
...
  * index: 4
	name: <alsa_input.usb-PaniRCorp_MicNode_123456-00.mono-fallback>
	driver: <module-alsa-card.c>
	flags: HARDWARE HW_MUTE_CTRL HW_VOLUME_CTRL DECIBEL_VOLUME LATENCY DYNAMIC_LATENCY
	state: RUNNING
	suspend cause: (none)
	priority: 9040
	volume: mono: 65536 / 100% / 0.00 dB
	        balance 0.00
	base volume: 64662 /  99% / -0.35 dB
	volume steps: 65537
	muted: no
	current latency: 2.54 ms
	max rewind: 0 KiB
	sample spec: s16le 1ch 16000Hz
	channel map: mono
	             Mono
	used by: 1
	linked by: 1
	configured latency: 24.99 ms; range is 0.50 .. 2000.00 ms
	card: 7 <alsa_card.usb-PaniRCorp_MicNode_123456-00>
	module: 31
	properties:
		alsa.resolution_bits = "16"
		device.api = "alsa"
		device.class = "sound"
		alsa.class = "generic"
		alsa.subclass = "generic-mix"
		alsa.name = "USB Audio"
		alsa.id = "USB Audio"
		alsa.subdevice = "0"
		alsa.subdevice_name = "subdevice #0"
		alsa.device = "0"
		alsa.card = "4"
		alsa.card_name = "MicNode"
		alsa.long_card_name = "PaniRCorp MicNode at usb-0000:04:00.4-1.3.2, full speed"
		alsa.driver_name = "snd_usb_audio"
		device.bus_path = "pci-0000:04:00.4-usb-0:1.3.2:1.0"
		sysfs.path = "/devices/pci0000:00/0000:00:08.1/0000:04:00.4/usb3/3-1/3-1.3/3-1.3.2/3-1.3.2:1.0/sound/card4"
		udev.id = "usb-PaniRCorp_MicNode_123456-00"
		device.bus = "usb"
		device.vendor.id = "cafe"
		device.vendor.name = "PaniRCorp"
		device.product.id = "4011"
		device.product.name = "MicNode"
		device.serial = "PaniRCorp_MicNode_123456"
		device.string = "hw:4"
		device.buffering.buffer_size = "64000"
		device.buffering.fragment_size = "32000"
		device.access_mode = "mmap+timer"
		device.profile.name = "mono-fallback"
		device.profile.description = "Mono"
		device.description = "MicNode Mono"
		module-udev-detect.discovered = "1"
		device.icon_name = "audio-card-usb"
	ports:
		analog-input-mic: Microphone (priority 8700, latency offset 0 usec, available: yes)
			properties:
				device.icon_name = "audio-input-microphone"
	active port: <analog-input-mic>
```

The latency and peformance of the emulated microphone is fine. Check by
recording from this device using Audacity.
