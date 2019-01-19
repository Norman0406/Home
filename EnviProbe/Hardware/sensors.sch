EESchema Schematic File Version 4
LIBS:EnviProbe-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title "Sensors"
Date "2019-01-19"
Rev "1.0"
Comp "Norman Link"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Sensor_Pressure:BMP280 U?
U 1 1 5C876622
P 5800 4200
AR Path="/5C876622" Ref="U?"  Part="1" 
AR Path="/5C866452/5C876622" Ref="U?"  Part="1" 
F 0 "U?" H 6029 4296 50  0000 L CNN
F 1 "BMP280" H 6029 4205 50  0000 L CNN
F 2 "Package_LGA:Bosch_LGA-8_2x2.5mm_P0.65mm_ClockwisePinNumbering" H 5800 3500 50  0001 C CNN
F 3 "https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-19.pdf" H 5800 4200 50  0001 C CNN
	1    5800 4200
	1    0    0    -1  
$EndComp
$Comp
L EnviProbe:HTU21D U?
U 1 1 5C876630
P 3800 1950
AR Path="/5C876630" Ref="U?"  Part="1" 
AR Path="/5C866452/5C876630" Ref="U?"  Part="1" 
F 0 "U?" H 3800 2917 50  0000 C CNN
F 1 "HTU21D" H 3800 2826 50  0000 C CNN
F 2 "QFN100P300X300X100-7N" H 3800 1950 50  0001 L BNN
F 3 "DFN-6 TE Connectivity" H 3800 1950 50  0001 L BNN
F 4 "CAT-HSC0004" H 3800 1950 50  0001 L BNN "Feld4"
F 5 "HTU21D" H 3800 1950 50  0001 L BNN "Feld5"
F 6 "https://www.te.com/usa-en/product-CAT-HSC0004.html?te_bu=Cor&te_type=disp&te_campaign=seda_glo_cor-seda-global-disp-prtnr-fy19-seda-model-bom-cta_sma-317_1&elqCampaignId=32493" H 3800 1950 50  0001 L BNN "Feld6"
F 7 "Unavailable" H 3800 1950 50  0001 L BNN "Feld7"
F 8 "Temp Sensor Digital Serial _2-Wire, I2C_ 6-Pin DFN" H 3800 1950 50  0001 L BNN "Feld8"
F 9 "TE Connectivity" H 3800 1950 50  0001 L BNN "Feld9"
F 10 "None" H 3800 1950 50  0001 L BNN "Feld10"
	1    3800 1950
	1    0    0    -1  
$EndComp
$Comp
L EnviProbe:MAX44009EDT+T U?
U 1 1 5C87663C
P 7150 1950
AR Path="/5C87663C" Ref="U?"  Part="1" 
AR Path="/5C866452/5C87663C" Ref="U?"  Part="1" 
F 0 "U?" H 7150 2517 50  0000 C CNN
F 1 "MAX44009EDT+T" H 7150 2426 50  0000 C CNN
F 2 "SON65P200X200X65-7N" H 7150 1950 50  0001 L BNN
F 3 "Light to Digital Ambient Light Sensor Digital O/P 0.045lux to 188000lux 6-Pin UTDFN EP T/R" H 7150 1950 50  0001 L BNN
F 4 "https://www.digikey.de/product-detail/en/maxim-integrated/MAX44009EDT-T/MAX44009EDT-TCT-ND/2606396?utm_source=snapeda&utm_medium=aggregator&utm_campaign=symbol" H 7150 1950 50  0001 L BNN "Feld4"
F 5 "MAX44009EDT+TCT-ND" H 7150 1950 50  0001 L BNN "Feld5"
F 6 "Maxim Integrated" H 7150 1950 50  0001 L BNN "Feld6"
F 7 "UDFN-6 Maxim Integrated" H 7150 1950 50  0001 L BNN "Feld7"
F 8 "MAX44009EDT+T" H 7150 1950 50  0001 L BNN "Feld8"
	1    7150 1950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 5C876643
P 4550 1900
AR Path="/5C876643" Ref="C?"  Part="1" 
AR Path="/5C866452/5C876643" Ref="C?"  Part="1" 
F 0 "C?" H 4665 1946 50  0000 L CNN
F 1 "0.1uF" H 4665 1855 50  0000 L CNN
F 2 "" H 4588 1750 50  0001 C CNN
F 3 "~" H 4550 1900 50  0001 C CNN
	1    4550 1900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5C87664A
P 4550 2750
AR Path="/5C87664A" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C87664A" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4550 2500 50  0001 C CNN
F 1 "GND" H 4555 2577 50  0000 C CNN
F 2 "" H 4550 2750 50  0001 C CNN
F 3 "" H 4550 2750 50  0001 C CNN
	1    4550 2750
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5C876650
P 4550 1100
AR Path="/5C876650" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C876650" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4550 950 50  0001 C CNN
F 1 "+3.3V" H 4565 1273 50  0000 C CNN
F 2 "" H 4550 1100 50  0001 C CNN
F 3 "" H 4550 1100 50  0001 C CNN
	1    4550 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 1100 4550 1450
Wire Wire Line
	4550 1450 4300 1450
Wire Wire Line
	4550 1450 4550 1750
Connection ~ 4550 1450
Wire Wire Line
	4550 2050 4550 2550
Wire Wire Line
	4550 2550 4300 2550
Wire Wire Line
	4550 2550 4550 2750
Connection ~ 4550 2550
$Comp
L power:+3.3V #PWR?
U 1 1 5C87665E
P 3150 1100
AR Path="/5C87665E" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C87665E" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 3150 950 50  0001 C CNN
F 1 "+3.3V" H 3165 1273 50  0000 C CNN
F 2 "" H 3150 1100 50  0001 C CNN
F 3 "" H 3150 1100 50  0001 C CNN
	1    3150 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5C876664
P 3150 1400
AR Path="/5C876664" Ref="R?"  Part="1" 
AR Path="/5C866452/5C876664" Ref="R?"  Part="1" 
F 0 "R?" H 3220 1446 50  0000 L CNN
F 1 "4.7k" H 3220 1355 50  0000 L CNN
F 2 "" V 3080 1400 50  0001 C CNN
F 3 "~" H 3150 1400 50  0001 C CNN
	1    3150 1400
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5C87666B
P 2850 1100
AR Path="/5C87666B" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C87666B" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 2850 950 50  0001 C CNN
F 1 "+3.3V" H 2865 1273 50  0000 C CNN
F 2 "" H 2850 1100 50  0001 C CNN
F 3 "" H 2850 1100 50  0001 C CNN
	1    2850 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5C876671
P 2850 1400
AR Path="/5C876671" Ref="R?"  Part="1" 
AR Path="/5C866452/5C876671" Ref="R?"  Part="1" 
F 0 "R?" H 2920 1446 50  0000 L CNN
F 1 "4.7k" H 2920 1355 50  0000 L CNN
F 2 "" V 2780 1400 50  0001 C CNN
F 3 "~" H 2850 1400 50  0001 C CNN
	1    2850 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 1100 3150 1250
Wire Wire Line
	2850 1100 2850 1250
Wire Wire Line
	3150 1550 3150 1750
Wire Wire Line
	3150 1750 3300 1750
Wire Wire Line
	3300 2250 2850 2250
Wire Wire Line
	2850 2250 2850 1550
Wire Wire Line
	3150 1750 2550 1750
Connection ~ 3150 1750
Wire Wire Line
	2850 2250 2550 2250
Connection ~ 2850 2250
$Comp
L EnviProbe:ADMP401 U?
U 1 1 5C876686
P 2500 4550
AR Path="/5C876686" Ref="U?"  Part="1" 
AR Path="/5C866452/5C876686" Ref="U?"  Part="1" 
F 0 "U?" H 2500 5117 50  0000 C CNN
F 1 "ADMP401" H 2500 5026 50  0000 C CNN
F 2 "ADMP401" H 2500 4550 50  0001 L BNN
F 3 "None" H 2500 4550 50  0001 L BNN
F 4 "ADMP401" H 2500 4550 50  0001 L BNN "Feld4"
F 5 "Unavailable" H 2500 4550 50  0001 L BNN "Feld5"
F 6 "InvenSense" H 2500 4550 50  0001 L BNN "Feld7"
F 7 "None" H 2500 4550 50  0001 L BNN "Feld8"
	1    2500 4550
	1    0    0    -1  
$EndComp
Text Label 2550 2250 0    50   ~ 0
SDA
Text Label 2550 1750 0    50   ~ 0
SCL
$Comp
L Device:R R?
U 1 1 5C87668F
P 6300 1500
AR Path="/5C87668F" Ref="R?"  Part="1" 
AR Path="/5C866452/5C87668F" Ref="R?"  Part="1" 
F 0 "R?" H 6370 1546 50  0000 L CNN
F 1 "4.7k" H 6370 1455 50  0000 L CNN
F 2 "" V 6230 1500 50  0001 C CNN
F 3 "~" H 6300 1500 50  0001 C CNN
	1    6300 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5C876696
P 6000 1500
AR Path="/5C876696" Ref="R?"  Part="1" 
AR Path="/5C866452/5C876696" Ref="R?"  Part="1" 
F 0 "R?" H 6070 1546 50  0000 L CNN
F 1 "4.7k" H 6070 1455 50  0000 L CNN
F 2 "" V 5930 1500 50  0001 C CNN
F 3 "~" H 6000 1500 50  0001 C CNN
	1    6000 1500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5C87669D
P 6000 1250
AR Path="/5C87669D" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C87669D" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 6000 1100 50  0001 C CNN
F 1 "+3.3V" H 6015 1423 50  0000 C CNN
F 2 "" H 6000 1250 50  0001 C CNN
F 3 "" H 6000 1250 50  0001 C CNN
	1    6000 1250
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5C8766A3
P 6300 1250
AR Path="/5C8766A3" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C8766A3" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 6300 1100 50  0001 C CNN
F 1 "+3.3V" H 6315 1423 50  0000 C CNN
F 2 "" H 6300 1250 50  0001 C CNN
F 3 "" H 6300 1250 50  0001 C CNN
	1    6300 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 2050 6300 1650
Wire Wire Line
	6000 2150 6000 1650
Wire Wire Line
	6000 1350 6000 1250
Wire Wire Line
	6300 1350 6300 1250
Wire Wire Line
	6300 2050 5450 2050
Wire Wire Line
	6000 2150 5450 2150
Text Label 5450 2050 0    50   ~ 0
SCL
Text Label 5450 2150 0    50   ~ 0
SDA
Wire Wire Line
	7750 1650 7950 1650
Wire Wire Line
	7950 1650 7950 1150
$Comp
L power:+3.3V #PWR?
U 1 1 5C8766B3
P 7950 1150
AR Path="/5C8766B3" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C8766B3" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 7950 1000 50  0001 C CNN
F 1 "+3.3V" H 7965 1323 50  0000 C CNN
F 2 "" H 7950 1150 50  0001 C CNN
F 3 "" H 7950 1150 50  0001 C CNN
	1    7950 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	7750 2350 7950 2350
Wire Wire Line
	7950 2350 7950 2600
$Comp
L power:GND #PWR?
U 1 1 5C8766BB
P 7950 2600
AR Path="/5C8766BB" Ref="#PWR?"  Part="1" 
AR Path="/5C866452/5C8766BB" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 7950 2350 50  0001 C CNN
F 1 "GND" H 7955 2427 50  0000 C CNN
F 2 "" H 7950 2600 50  0001 C CNN
F 3 "" H 7950 2600 50  0001 C CNN
	1    7950 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	6550 2050 6300 2050
Connection ~ 6300 2050
Wire Wire Line
	6550 2150 6000 2150
Connection ~ 6000 2150
Wire Wire Line
	7750 1850 7950 1850
Wire Wire Line
	7750 2250 7950 2250
Wire Wire Line
	6550 1850 6400 1850
NoConn ~ 7950 1850
NoConn ~ 7950 2250
NoConn ~ 6400 1850
Text HLabel 1000 1050 0    50   Input ~ 0
VCC
Text HLabel 1000 1150 0    50   Input ~ 0
GND
Text HLabel 1000 1250 0    50   Input ~ 0
SCL
Text HLabel 1000 1350 0    50   Input ~ 0
SDA
Text Label 1200 1250 0    50   ~ 0
SCL
Text Label 1200 1350 0    50   ~ 0
SDA
$Comp
L power:+3.3V #PWR?
U 1 1 5C8784C7
P 1600 950
F 0 "#PWR?" H 1600 800 50  0001 C CNN
F 1 "+3.3V" H 1615 1123 50  0000 C CNN
F 2 "" H 1600 950 50  0001 C CNN
F 3 "" H 1600 950 50  0001 C CNN
	1    1600 950 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5C878502
P 1600 1250
F 0 "#PWR?" H 1600 1000 50  0001 C CNN
F 1 "GND" H 1605 1077 50  0000 C CNN
F 2 "" H 1600 1250 50  0001 C CNN
F 3 "" H 1600 1250 50  0001 C CNN
	1    1600 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 1250 1200 1250
Wire Wire Line
	1000 1150 1600 1150
Wire Wire Line
	1600 1150 1600 1250
Wire Wire Line
	1000 1050 1600 1050
Wire Wire Line
	1600 1050 1600 950 
Wire Wire Line
	1000 1350 1200 1350
$EndSCHEMATC
