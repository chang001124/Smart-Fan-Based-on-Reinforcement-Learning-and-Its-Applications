<?php
 $servername = "localhost";
 $username = "root";
 $password = "";
 $dbname = "smart_fan";
$db = new mysqli($servername, $username, $password, $dbname);
$count=$_GET['count'];
$user=$_GET['username'];
$query = "SELECT $count FROM $user";
$result = $db->query($query);
$data = array();
while ($row = $result->fetch_row()) {
    $data[] = $row;
}
echo json_encode($data);
?>
