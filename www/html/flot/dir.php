<html>
 <head>
  <link rel="SHORTCUT ICON" href="../icon.png" />
 </head>
 <body>
  <h1><img src="../icon.png" width="100">/var/www/dat/</h1>

<a href="../index.html">indietro</a>
<br>  <br>
<?php
$a=time();
$b=date('Y-m-d', $a).'.dat';
            echo "<font color='red' face='verdana' size='4'><b><a href=flot.php?filename=".$b.">".$b."</a></font></b></br></br>";


if ($handle = opendir('/var/www/dat')) {
    while (false !== ($entry = readdir($handle))) {
        if ($entry != "." && $entry != "..") {
            echo "<a href=flot.php?filename=".$entry.">".$entry."</a></br>";
        }
    }
    closedir($handle);
}
?>
 </body>
</html>
