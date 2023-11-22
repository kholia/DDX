<Qucs Schematic 0.0.19>
<Properties>
  <View=0,-300,906,1080,1.94872,0,929>
  <Grid=10,10,1>
  <DataSet=Wider-LPF-QUCS.dat>
  <DataDisplay=Wider-LPF-QUCS.dpl>
  <OpenDisplay=1>
  <Script=Wider-LPF-QUCS.m>
  <RunScript=0>
  <showFrame=0>
  <FrameText0=Title>
  <FrameText1=Drawn By:>
  <FrameText2=Date:>
  <FrameText3=Revision:>
</Properties>
<Symbol>
</Symbol>
<Components>
  <Pac P1 1 260 340 18 -26 0 1 "1" 1 "50 Ohm" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0>
  <GND * 1 260 370 0 0 0 0>
  <GND * 1 370 370 0 0 0 0>
  <L L1 1 440 260 -26 10 0 0 "360nH" 1 "" 0>
  <GND * 1 510 370 0 0 0 0>
  <L L2 1 580 260 -26 10 0 0 "360nH" 1 "" 0>
  <GND * 1 650 370 0 0 0 0>
  <Pac P2 1 760 340 18 -26 0 1 "2" 1 "50 Ohm" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0>
  <GND * 1 760 370 0 0 0 0>
  <.SP SP1 1 270 440 0 67 0 0 "log" 1 "3.3MHz" 1 "45MHz" 1 "201" 1 "no" 0 "1" 0 "2" 0 "no" 0 "no" 0>
  <Eqn Eqn1 1 490 450 -28 15 0 0 "dBS21=dB(S[2,1])" 1 "dBS11=dB(S[1,1])" 1 "yes" 0>
  <C C1 1 370 340 17 -26 0 1 "100pF" 1 "" 0 "neutral" 0>
  <C C2 1 510 340 17 -26 0 1 "180pF" 1 "" 0 "neutral" 0>
  <C C3 1 650 340 17 -26 0 1 "100pF" 1 "" 0 "neutral" 0>
</Components>
<Wires>
  <260 260 260 310 "" 0 0 0 "">
  <260 260 370 260 "" 0 0 0 "">
  <370 260 370 310 "" 0 0 0 "">
  <510 260 510 310 "" 0 0 0 "">
  <650 260 650 310 "" 0 0 0 "">
  <370 260 410 260 "" 0 0 0 "">
  <470 260 510 260 "" 0 0 0 "">
  <510 260 550 260 "" 0 0 0 "">
  <610 260 650 260 "" 0 0 0 "">
  <760 260 760 310 "" 0 0 0 "">
  <650 260 760 260 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text 600 440 12 #000000 0 "Chebyshev low-pass filter \n 30MHz+ cutoff, pi-type, \n impedance matching 50 Ohm">
</Paintings>
