<QucsStudio Schematic 4.3.1>
<Properties>
View=0,-360,1069,980,1.95076,0,732
Grid=10,10,1
DataSet=*.dat
DataDisplay=
OpenDisplay=1
showFrame=0
FrameText0=Title
FrameText1=Drawn By:
FrameText2=Date:
FrameText3=Revision:
</Properties>
<Symbol>
</Symbol>
<Components>
Pac P1 1 40 260 18 -26 0 0 "1" 1 "50" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 40 290 0 0 0 0
GND * 1 150 290 0 0 0 0
GND * 1 290 290 0 0 0 0
GND * 1 430 290 0 0 0 0
Pac P2 1 540 260 18 -26 0 0 "2" 1 "50" 1 "0" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 540 290 0 0 0 0
.SP SP1 1 40 360 0 67 0 0 "log" 1 "3MHz" 1 "60MHz" 1 "500" 1 "no" 0 "1" 0 "2" 0 "none" 0
L L1 1 220 180 -26 10 0 0 "360nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
L L2 1 360 180 -26 10 0 0 "360nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
C C1 1 150 260 17 -26 0 1 "120pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C2 1 290 260 17 -26 0 1 "200pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C3 1 430 260 17 -26 0 1 "120pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
</Components>
<Wires>
40 180 40 230 "" 0 0 0 ""
40 180 150 180 "" 0 0 0 ""
150 180 150 230 "" 0 0 0 ""
290 180 290 230 "" 0 0 0 ""
430 180 430 230 "" 0 0 0 ""
150 180 190 180 "" 0 0 0 ""
250 180 290 180 "" 0 0 0 ""
290 180 330 180 "" 0 0 0 ""
390 180 430 180 "" 0 0 0 ""
540 180 540 230 "" 0 0 0 ""
430 180 540 180 "" 0 0 0 ""
</Wires>
<Diagrams>
<Rect 709 250 360 220 31 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 1 315 0 225 "" "" "">
	<Legend 10 -100 0>
	<"dB(S[1,1])" "" #0000ff 0 3 0 0 0 0 "">
	<"dB(S[2,1])" "" #ff0000 0 3 0 0 0 0 "">
</Rect>
<Smith 709 550 220 220 31 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 1 315 0 225 "" "" "">
	<Legend 10 -100 0>
	<"S[1,1]" "" #0000ff 0 3 0 0 0 0 "">
</Smith>
</Diagrams>
<Paintings>
Text 200 360 16 #000000 0 low-pass filter, 30MHz cutoff \n 5^{th} order Chebyshev , PI-type, \n impedance 50 ohms
</Paintings>
