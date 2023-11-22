Without any filter @ 28 MHZ -> Full AM audio (792 kHz AIR Pune) is replicated
with high fidelity ;)

With my 3-Pole BCI filter @ 28 MHz -> FT8 traffic is heard and decoded, AM
voices CANNOT be hear for the most part BUT the audio output has loud screechy,
weird artifacts! It is not pleasant to listen to at all.

With professional "Morgan System SurgeStop" BPF @ 28 MHz -> The loud screechy,
weird audio artifacts are gone 100%! FT8 traffic is heard very clearly but
the audio output is quite feeble - which is totally fine as we just need to
amplify the audio output more.

With professional "Morgan System SurgeStop" BCI filter (3.5 MHz model), we
can still hear the loud screechy, weird artifacts! Same as the 3-Pole homebrew
BCI filter!

The question now is: Can the 7-pole VK3YE BCI RX filter also provide similar
receive performance as the `SurgeStop` BPF?
VK3YE BCI filter -> https://www.youtube.com/watch?v=fgQenFJOjr4

Update: With 7-pole VK3YE BCI RX filter, we still hear the loud screechy, weird
audio artifacts, same as the professional "Morgan System SurgeStop" BCI filter!

Update 2: On 15th-November-2023, I couldn't hear 28.074 MHz with "RSP1 Clone
SDR" at all! The BW was set to widest 14 MHz! After attaching the Pro-BPF
filter, I could hear FT8 tunes faintly. After reducing the BW to 1 MHz, the
same FT8 tunes became super loud! Conclusion: We can't really have best RX
performance with a super-wide BW (duh!).

Conclusion 1: BCI is not the only problem! We have other types of interferences
going on around 28 MHz - from the nearby OTH radar and perhaps from strong
shortwave station(s) it seems!

Conclusion 2: We surely need a BPF filter bank for crystal clear audio, and
high performance!

Finding: The QRP Labs' BPF filters also work just fine.
