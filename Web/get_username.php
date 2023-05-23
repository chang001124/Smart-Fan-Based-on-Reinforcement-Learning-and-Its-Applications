<?php

// 資料庫連接設定
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "smart_fan";
$conn = new mysqli($servername, $username, $password, $dbname);

// 檢查連接是否成功
if ($conn->connect_error) {
    die("資料庫連接失敗: " . $conn->connect_error);
}


$query = "SELECT username FROM account WHERE timet = (SELECT max(timet) from account)";
$result = $conn->query($query);
$data = array();
while ($row = $result->fetch_row()) {
    $data[] = $row;
}
echo json_encode($data);

// 關閉資料庫連接
$conn->close();

?>