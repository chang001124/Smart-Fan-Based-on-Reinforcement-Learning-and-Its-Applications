<?php

// 資料庫連接設定
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "smart_fan";

// 建立與資料庫的連接
$conn = new mysqli($servername, $username, $password, $dbname);

// 檢查連接是否成功
if ($conn->connect_error) {
    die("資料庫連接失敗: " . $conn->connect_error);
}
echo "connect<br>";
// 取得用戶輸入的帳號
$username = $_POST["name"];
$password = $_POST["password"];
// 在資料庫中檢查帳號是否存在
$sql = "SELECT * FROM account WHERE username = '$username'";
$result = $conn->query($sql);
// $a=$conn->query("SELECT IF(username = '$username' AND password = '$password', 1, 0) AS results FROM account")-> num_rows;
if ($result->num_rows > 0) {
    if($conn->query("SELECT * FROM account WHERE username = '$username' AND password = '$password'")-> num_rows){
        //$conn->query("UPDATE users SET last_login_time = NOW() WHERE username = 'user1';");
        $sql = "UPDATE account SET timet = NOW() WHERE username = '$username'";
        if (mysqli_query($conn, $sql))
          echo  "hellow " . $username . $password ;
        
    }
    // 如果帳號已經存在，返回"重複"
    else{
        echo "already exists<br>";
        header('location:http://127.0.0.1/smart_fan/login2.html');
    }
    
} else {
    // 如果帳號不存在，返回"可用"
    if (isset($_POST["name"]) && isset($_POST["password"])) {
        //   // 獲取表單提交的數據
           $sql = "INSERT INTO account (username, password,timet) VALUES ('$username', '$password',now())";
        
          if (mysqli_query($conn, $sql)) {
            $sql = "CREATE TABLE `$username` (
              `id` int(11) NOT NULL DEFAULT 100,
              `a1` int(100) NOT NULL DEFAULT 100,
              `a2` int(100) NOT NULL DEFAULT 100,
              `a3` int(100) NOT NULL DEFAULT 100,
              `a4` int(100) NOT NULL DEFAULT 100,
              `a5` int(100) NOT NULL DEFAULT 100,
              `a6` int(100) NOT NULL DEFAULT 100,
              `a7` int(100) NOT NULL DEFAULT 100,
              `a8` int(100) NOT NULL DEFAULT 100,
              `a9` int(100) NOT NULL DEFAULT 100,
              `a10` int(100) NOT NULL DEFAULT 100,
              `a11` int(100) NOT NULL DEFAULT 100,
              `a12` int(100) NOT NULL DEFAULT 100,
              `a13` int(100) NOT NULL DEFAULT 100,
              `a14` int(100) NOT NULL DEFAULT 100,
              `a15` int(100) NOT NULL DEFAULT 100,
              `a16` int(100) NOT NULL DEFAULT 100,
              `a17` int(11) NOT NULL DEFAULT 100,
              `a18` int(11) NOT NULL DEFAULT 100,
              `a19` int(11) NOT NULL DEFAULT 100,
              `a20` int(11) NOT NULL DEFAULT 100,
              `a21` int(11) NOT NULL DEFAULT 100,
              `a22` int(11) NOT NULL DEFAULT 100,
              `a23` int(11) NOT NULL DEFAULT 100,
              `a24` int(11) NOT NULL DEFAULT 100,
              `a25` int(11) NOT NULL DEFAULT 100,
              `a26` int(11) NOT NULL DEFAULT 100,
              `a27` int(11) NOT NULL DEFAULT 100
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;";
            if(mysqli_query($conn, $sql)) {
                $sql="ALTER TABLE `$username`ADD PRIMARY KEY (`id`);";
                if(mysqli_query($conn, $sql)) {
                  $sql="ALTER TABLE `$username` MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;";
                  if(mysqli_query($conn, $sql)) {
                    for($i=1;$i<=63;$i++){
                      $sql = "INSERT INTO $username (a1)VALUES ('100')";
                      if(mysqli_query($conn, $sql)){
                        //echo "created successfully";
                      }
                      else{
                        echo "Error: " . $sql . mysqli_error($conn);
                      }
                   }
                    echo  "hellow " . $username . $password ;
                }
              }
            }
          } else {
            echo "Error: " . $sql . "<br>" . mysqli_error($conn);
          }
         }
         else{
           echo ("connect error");
         }
}

// 關閉資料庫連接
$conn->close();

?>