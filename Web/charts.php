<?php
 $servername = "localhost";
 $username = "root";
 $password = "";
 $dbname = "smart_fan_charts";
 // Create connection
 $conn = mysqli_connect($servername, $username, $password, $dbname);
 // Check connection
 $temp = $_GET["temp"];
 $fan_speed = $_GET["fan_speed"];
 if (!$conn) {
   die("Connection failed: " . mysqli_connect_error());
 }
 
$sql = "INSERT INTO stupid_fan (temp,fan_speed)VALUES ('$temp','$fan_speed')";
if(mysqli_query($conn, $sql)){
     echo "created successfully";
}
else{
    echo "Error: " . $sql . mysqli_error($conn);
}
 
 mysqli_close($conn);

?>