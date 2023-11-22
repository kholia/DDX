<QucsStudio Schematic 4.3.1>
<Properties>
View=0,-237,1316,860,2.30544,375,1030
Grid=10,10,1
DataSet=*.dat
DataDisplay=*.dpl
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
Pac P1 1 480 450 18 -26 0 0 "1" 1 "50" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 480 480 0 0 0 0
GND * 1 630 480 0 0 0 0
GND * 1 750 480 0 0 0 0
GND * 1 870 480 0 0 0 0
Pac P2 1 960 450 18 -26 0 0 "2" 1 "50" 1 "0" 0 "1 GHz" 0 "26.85" 0 "SUBCLICK" 0
GND * 1 960 480 0 0 0 0
C C1 1 510 370 -27 -52 0 0 "47pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
L L1 1 630 450 8 -26 0 1 "100nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
C C2 1 600 450 -8 46 0 1 "240pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C3 1 630 370 -27 -52 0 0 "16pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C5 1 750 370 -27 -52 0 0 "16pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C7 1 870 370 -27 -52 0 0 "47pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
C C6 1 840 450 -8 46 0 1 "240pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
L L3 1 870 450 8 -26 0 1 "100nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
L L2 1 750 450 8 -26 0 1 "100nH" 1 "0" 0 "" 0 "SELF-WE-PD3S" 0
C C4 1 720 450 -8 46 0 1 "270pF" 1 "0" 0 "" 0 "neutral" 0 "SMD0603" 0
.SP SP1 1 500 550 0 67 0 0 "log" 1 "26MHz" 1 "40MHz" 1 "601" 1 "no" 0 "1" 0 "2" 0 "none" 0
</Components>
<Wires>
600 370 600 420 "" 0 0 0 ""
720 370 720 420 "" 0 0 0 ""
840 370 840 420 "" 0 0 0 ""
600 420 630 420 "" 0 0 0 ""
600 480 630 480 "" 0 0 0 ""
660 370 720 370 "" 0 0 0 ""
720 420 750 420 "" 0 0 0 ""
720 480 750 480 "" 0 0 0 ""
780 370 840 370 "" 0 0 0 ""
840 420 870 420 "" 0 0 0 ""
840 480 870 480 "" 0 0 0 ""
900 370 960 370 "" 0 0 0 ""
960 370 960 420 "" 0 0 0 ""
540 370 600 370 "" 0 0 0 ""
480 370 480 420 "" 0 0 0 ""
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
Text 660 550 16 #000000 0 band-pass filter, 28MHz...29.7MHz \n 3^{th} order Chebyshev  \n impedance 50 \\Omega
</Paintings>
