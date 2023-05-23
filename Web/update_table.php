<?php
 $servername = "localhost";
 $username = "root";
 $password = "";
 $dbname = "smart_fan";
 // Create connection
 $conn = mysqli_connect($servername, $username, $password, $dbname);
 // Check connection
 if (!$conn) {
   die("Connection failed: " . mysqli_connect_error());
 }
 //$json_data = file_get_contents('php://input');
 $json_data = $_GET['array1'];
 $count=$_GET['count'];
 $user=$_GET['username'];
 $data = json_decode($json_data, true);
 
if($data != null){
    echo "have data";
}
else{
    echo "NO data";
}
$num=1;
 foreach ($data as $row) {
    foreach ($row as $value) {

      $sql = "UPDATE $user SET $count='$value' WHERE id=$num ";
      $num=$num+1;
      if(mysqli_query($conn, $sql)){
        echo "1";
      }
      else{
         echo "Error: " . $sql . mysqli_error($conn);
      }
    }
  }

 
 mysqli_close($conn);

?>
