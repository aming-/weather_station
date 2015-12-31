<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
 <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<meta http-equiv="refresh" content="30" >
	<link rel="SHORTCUT ICON" href="../icon.png" />

    <title>Flot H/T/HTI</title>
    <link href="layout.css" rel="stylesheet" type="text/css">
    <!--[if lte IE 8]><script language="javascript" type="text/javascript" src="./excanvas.min.js"></script><![endif]-->
    <script language="javascript" type="text/javascript" src="./jquery.js"></script>
    <script language="javascript" type="text/javascript" src="./jquery.flot.js"></script>
 </head>
    <body>
	
<?php
	$filename = $_REQUEST['filename'];
	$handle = fopen("/var/www/dat/".$filename, "r");
	while (($buffer = fgets($handle)) !== false) 
	{	
		$splitarray = explode(";",$buffer);
	}
	fclose($handle);
	
	$humidity=$splitarray[1];
	$temperature=$splitarray[2];
	$thi=$splitarray[3];
	$temp=$splitarray[4];
	$pressure=$splitarray[5];
	$sealvl=$splitarray[6];
	
	switch($thi)
	{
		case ($thi < -40):
			$clima="Iperglaciale";
			break;
		case ($thi > -40 and $thi <= -20):
			$clima="Glaciale";
			break;
		case ($thi > -20 and $thi <= -10):
			$clima="Estremamente freddo";
			break;
		case ($thi > -10 and $thi <= -1.8):
			$clima="Molto freddo";
			break;
		case ($thi > -1.8 and $thi <= 13):
			$clima="Freddo";
			break;
		case ($thi > 13 and $thi <= 15):
			$clima="Fresco";
			break;
		case ($thi > 15 and $thi <= 20):
			$clima="Confortevole";
			break;
		case ($thi > 20 and $thi <= 26.5):
			$clima="Caldo";
			break;
		case ($thi > 26.5 and $thi <= 30):
			$clima="Molto caldo";
			break;
		case ($thi > 30):
			$clima="Torrido";
			break;
		default:
			$clime="";
	}
	
	
	$filename = substr($filename,0,10);
    echo "<b>".$filename."</b><br><font color='red' face='verdana' size='4'><b>H=".$humidity."%  T=".$temperature."&#186;C P=".$pressure."hPa <br>THI=".$thi."&#186;C - ".$clima."<br></b></font>";
?>    
<b>Humidity %</b>
    <div id="placeholder1" style="width:1200px;height:300px;"></div>
<b>Temperature &#186;C</b>
    <div id="placeholder2" style="width:1200px;height:300px;"></div>
<b>Thermohygrometric Index &#186;C</b>
    <div id="placeholder3" style="width:1200px;height:300px;"></div>
<b>Pressure hPa</b>
    <div id="placeholder4" style="width:1200px;height:300px;"></div>

<script type="text/javascript">
$(function () {
    var d1 = [];
    var d2 = [];
    var d3 = [];
	var d4 = [];
	
<?php
	$filename = $_REQUEST['filename'];
	$handle = fopen("/var/www/dat/".$filename, "r");
	while (($buffer = fgets($handle)) !== false) 
	{
	$splitarray = explode(";",$buffer);

	$stamp=($splitarray[0] + 3600)*1000;
	$humidity=$splitarray[1];
	$temperature=$splitarray[2];
	$thi=$splitarray[3];
	$temp=$splitarray[4];
	$pressure=$splitarray[5];
	$sealvl=$splitarray[6];
	
	
	
	$i=$i+1;
        
    echo 'd1.push(['.$stamp.', '.$humidity.']);';
    echo 'd2.push(['.$stamp.', '.$temperature.']);';
    echo 'd3.push(['.$stamp.', '.$thi.']);';
    echo 'd4.push(['.$stamp.', '.$pressure.']);';
	
	}
	fclose($handle);
?>    
    $.plot($("#placeholder1"), [ d1 ], { xaxis: { mode: "time" } });
    $.plot($("#placeholder2"), [ d2 ], { xaxis: { mode: "time" } });
    $.plot($("#placeholder3"), [ d3 ], { xaxis: { mode: "time" } });
    $.plot($("#placeholder4"), [ d4 ], { xaxis: { mode: "time" } });
	
});
</script>
</br>
<a href="dir.php">indietro</a>

 </body>
</html>
