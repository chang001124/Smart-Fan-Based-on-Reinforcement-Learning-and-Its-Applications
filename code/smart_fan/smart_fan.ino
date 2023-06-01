#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESP8266.h"
#include <ArduinoJson.h>
#define ONE_WIRE_BUS 5 // 1-Wire 總線接口引腳
#define fan_pin 8 // 電風扇 PWM 引腳
#define A 21
#define S 81
#define ALPHA 0.1 // Q-learning 學習率
#define GAMMA 0.9 // Q-learning 折扣因子
#define episode 3000
#define SSID        "lolsfd"
#define PASSWORD    "0975820030"
#define HOST_NAME   "172.20.10.5"
#define HOST_PORT   (80)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
const int button_cold = 2;
const int button_hot = 3;
const int ledPin = 13;
const char* username;
int repeatCount = 3;
int fan_speed = 128;
int a_table[S];
float temperature = 0.0;
float upper = -1.0;
float lower = -1.0; 
float goal = 0.0;
float rt=23;//室溫
ESP8266 wifi(Serial3);


void setup() {
  Serial.begin(9600);
  sensors.begin();
  pinMode(button_cold,INPUT); 
  pinMode(button_hot,INPUT);
  pinMode(fan_pin,OUTPUT);
  pinMode(ledPin, OUTPUT);
  int count=80;
  for(int i=0;i<41;i++){
    a_table[i]=count;
    count-=2;
  }
  for(int i=41;i<S;i++){
    a_table[i]=count;
    count-=2;
  }
    Serial.print(F("setup begin\r\n"));

    Serial.print(F("FW Version:"));
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print(F("to station + softap ok\r\n"));
    } else {
        Serial.print(F("to station + softap err\r\n"));
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print(F("Join AP success\r\n"));

        Serial.print(F("IP:"));
        Serial.println(wifi.getLocalIP().c_str());       
    } else {
        Serial.print(F("Join AP failure\r\n"));
    }
    
    if (wifi.disableMUX()) {
        Serial.print(F("single ok\r\n"));
    } else {
        Serial.print(F("single err\r\n"));
    }
    
    Serial.print(F("setup end\r\n"));
  //show_table();
}
//----------------------------------------------------
void get_username(){     //get user name char* username
  uint8_t buffer[100] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print(F("create tcp ok\r\n"));
    } else {
        Serial.print(F("create tcp err\r\n"));
    }
    char *hello =("GET /smart_fan/get_username.php\r\n");//   HTTP/1.1\r\n Host: 172.20.10.5\r\n Connection: close\r\n
    wifi.send((const uint8_t*)hello, strlen(hello));
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    /*if (len > 0) {
        //Serial.print(F("Received:["));
        for(uint32_t i = 0; i < len; i++) {  
            //username[i] = (char)buffer[i];
            //Serial.print(username[i]);
            Serial.print(buffer[i]);
        }
        //Serial.print(F("]\r\n"));
    }*/
    //DynamicJsonDocument doc(500);
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc,buffer);
    username = doc[0][0];
//    Serial.print(username);
    
    /*if (wifi.releaseTCP()) {
        Serial.print(F("release tcp ok\r\n"));
    } else {
        Serial.print(F("release tcp err\r\n"));
    }*/
    doc.clear();
}
void getdata(int test_array[S][A], char* use){
    uint8_t buffer[1024] = {0};
    //Serial.print(F("get data\r\n"));
    
    int num_rows=0;
    for(int count=1;count<=27;count++){
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
          Serial.print(F("create tcp ok\r\n"));
      } else {
          Serial.print(F("create tcp err\r\n"));
      }
      char *hello =("GET /smart_fan/gettable.php?count=a");//   HTTP/1.1\r\n Host: 172.20.10.5\r\n Connection: close\r\n
      String c = String(count);//username=who
      char *tail2 = ("&username=");
      char *tail = "\r\n";
      String all = hello+c+tail2+use+tail;
      const char *url_complete = all.c_str();
      wifi.send((const uint8_t*)url_complete, strlen(url_complete));
      uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    
      /*if (len > 0) {
          Serial.println(len);
          Serial.print("Received:[");
          for(int i = 0; i < len; i++) {
              Serial.print(buffer[i]);
          }
          Serial.print("]\r\n");
      }*/
      
      // 解析JSON字串
      DynamicJsonDocument doc(1024);
      //StaticJsonDocument<1000> doc;
      DeserializationError error = deserializeJson(doc,buffer);

      int num=0;
      for(int j=num_rows;j<num_rows+3;j++){
        for(int k=0;k<A;k++){
          const char* data1 = doc[0][0];
          data1=doc[num][0];
          int l = atoi(data1); 
          //Serial.println("Num"+String(num));
          num++;
          //Serial.println(l);
          test_array[j][k]=l;

        }
      }
      
      num_rows+=3;
    delay(100);
      doc.clear();
  
  
      
  }
  /*if (wifi.releaseTCP()) {
          Serial.print(F("release tcp ok\r\n"));
      } else {
          Serial.print(F("release tcp err\r\n"));
      }*/
}

//-----------------------------------------------------------------------------------
void set_table(int number,int a,int q_table[][A]){// fanspeed index
  int i;
  if(a%2!=0)
    a+=1;
  //Serial.println(a);
  for(i=0;i<S;i++){
    if(a_table[i]==a)
      break;
  }
  //Serial.println(i);
  if(number>0){
    for(int j=number/10+10;j<A;j++)
      q_table[i][j]+=10;
  }
  else{
    for(int j=6;j>=0;j--)
      q_table[i][j]+=10;
  }
}
//---------------------------------------------------------------------------
void update_table(int test_array[S][A],char* use){
  int num_rows = 0;
  uint8_t buffer[1024] = {0};    
    // 將二維陣列編碼為JSON格式
    for(int count=1;count<=27;count++){
      //StaticJsonDocument json_doc(1024);
      StaticJsonDocument<1000> json_doc;
      JsonArray json_rows = json_doc.to<JsonArray>();
      for (int i = num_rows; i < num_rows+3; i++) {
        JsonArray json_cols = json_rows.createNestedArray();
        for (int j = 0; j < 21; j++) {
          json_cols.add(test_array[i][j]);
        }
      }
      num_rows += 3;
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
          Serial.print(F("create tcp ok\r\n"));
      } else {
          Serial.print(F("create tcp err\r\n"));
      }
      String json_string;
      serializeJson(json_rows,json_string);
      int len = json_string.length();

   
      String l = String(len);
      String C=String(count);

      char *head = ("GET /smart_fan/update_table.php?array1=");
      char *mid0=("&count=a");
      char *mid1 = ("\r\nContent-Type: application/json\r\nContent-Length:");
      char *mid2 = ("&username=");
      char *mid = ("\r\n");
      char *tail = ("\r\n\r\n");
      //String all = head +l+ mid + json_string + tail;
      String all = head + json_string + mid0 + C +mid2+use+ mid1 + l + tail;
      const char *url_complete = all.c_str();
      //char *url_complete =("GET /table1.php?id=51&name=1550\r\n");
      //char *url_complete =("GET /table1.php?id=51&name=1550\r\n");
      //Serial.println(url_complete);
      wifi.send((const uint8_t*)url_complete, strlen(url_complete));
      /*uint32_t leng = wifi.recv(buffer, sizeof(buffer), 100000);
      if (leng > 0) {
          Serial.print(F("Received:["));
          for(uint32_t i = 0; i < leng; i++) {
              Serial.print((char)buffer[i]);
          }
          Serial.print(F("]\r\n"));
      }*/
      delay(150);
      json_doc.clear();
  }
  /*if (wifi.releaseTCP()) {
          Serial.print(F("release tcp ok\r\n"));
      } else {
          Serial.print(F("release tcp err\r\n"));
      }*/
      
}
//------------------------------------------------------------------------
int find_max_value(int row,int q_table[S][A]){          
    int Max = -1;
    for(int i=0;i<A;i++){
      if(q_table[row][i]>Max){
        Max = q_table[row][i];
      }
    }
    return Max;  
}
int find_max_action(int row,int q_table[S][A]){
    int Max = -1,i,action;
    for(i=0;i<A;i++){
      if(q_table[row][i]>Max){
        Max = q_table[row][i];
      }
    }
    
    if(Max == 0){
        action = random(0,20);
    }
    else{
      action = i;
    }
    if(action >= 10){
         action = (action - 10) * 10;
    }
    else {
         action = (10 - action) * 10;
    }
    
    return action; 
}
void clear_all(int arr[]){
  int i;
  for(i=0;i<A;i++){
    arr[i] = -1;
  }
}
int find_max_idx(int row,int q_table[S][A]){
    int Max = q_table[row][0],i,idx,max_action[A],k=0;
    clear_all(max_action);
    //clear_all(max_action);
    for(i=0;i<A;i++){
      if(q_table[row][i]>Max){
        k=0;
        clear_all(max_action);
        max_action[k++] = i;
        Max = q_table[row][i];
        //idx=i;
      }
      else if(q_table[row][i]==Max){
        max_action[k++] = i;
      }
    }
    
    if(k>0){
      k--;
    }
   //Serial.println("K:"+String(k));
    idx = max_action[random(0,k)];
    //Serial.println("")
    return idx; 
}
bool is_good(float before,float after){
    float a,b;
    a = before - goal;
    b = after - goal;
    if(a>=b){
      return true;
    }
    return false;
}
int check_index(float temperature,float goat){
      int a=round(((temperature-goat)*10)),i;
      if(a%2!=0)
        a+=1;
      for(i=0;i<S;i++){
        if(a_table[i]==a){
          return i;
        }  
      }
      return -1; 
}
void show_table(int q_table[S][A]){
  for(int i=0;i<S;i++){
    Serial.print(i+String(" "));
    for(int j=0;j<A;j++){
       Serial.print(q_table[i][j]+String(" "));
    }
    Serial.println();
  }
}
void send_chart(float goat,int fan_speed){
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
          Serial.print(F("create tcp ok\r\n"));
      } else {
          Serial.print(F("create tcp err\r\n"));
      }
     String l = String(goat);
     String C=String(fan_speed);
    char *hello =("GET /smart_fan/charts.php?temp=");//   HTTP/1.1\r\n Host: 172.20.10.5\r\n Connection: close\r\n
    char *a=("&fan_speed=");
    char *cc=("\r\n");
    String all =hello+l+a+C+cc;
    const char *url_complete = all.c_str();
    wifi.send((const uint8_t*)url_complete, strlen(url_complete));
    //uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    /*if (len > 0) {
        //Serial.print(F("Received:["));
        for(uint32_t i = 0; i < len; i++) {  
            //username[i] = (char)buffer[i];
            //Serial.print(username[i]);
            Serial.print(buffer[i]);
        }
        //Serial.print(F("]\r\n"));
    }*/
    
    /*if (wifi.releaseTCP()) {
        Serial.print(F("release tcp ok\r\n"));
    } else {
        Serial.print(F("release tcp err\r\n"));
    }*/
}

void loop() {
  get_username();
  char* use = (char*)username;
  char target[20]; 
  float feel[2],cold=0,hot=0;
  strcpy(target, use);
  Serial.println(use);
  sensors.requestTemperatures();//偵測溫度
  Serial.println(sensors.getTempCByIndex(0));
  rt=sensors.getTempCByIndex(0);
  Serial.println(String("rt=")+rt);
  int q_table[S][A],set_temp[6],cnt=0;
  getdata(q_table,target);
  show_table(q_table);
  for(int i=0;i<6;i++)
    set_temp[i]=0;
  //show_table(q_table);
  analogWrite(fan_pin,fan_speed);
  //model 1 START
    int count=0;
    for (int i = 0; i < repeatCount; i++) {               
    //Serial.println(String("fan_speed:")+fan_speed);
//    Serial.print("5");
//    delay(1000);
//    Serial.print("4");
//    delay(1000);
//    Serial.print("3");
//    delay(1000);
    Serial.print("2");
    delay(1000);
    Serial.println("1");
    delay(1000);
    digitalWrite(ledPin, HIGH);
    while(true){                                      //1->hot 0->cold
        if (digitalRead(button_cold) == 0) {
        Serial.println(F("button_cold Pressed"));
        fan_speed -= 40;
        set_temp[count]=-40;
        lower = fan_speed;
        break;
      } else if (digitalRead(button_hot) == 0) {
        Serial.println(F("button_hot Pressed"));
        fan_speed += 40;
        set_temp[count]=40;
        upper = fan_speed;
        break;
      }
    }
    sensors.requestTemperatures();//偵測溫度
    Serial.println(sensors.getTempCByIndex(0));
    if(set_temp[count]==40){
      feel[1]=sensors.getTempCByIndex(0);
      if(hot==0)
        hot = sensors.getTempCByIndex(0);
      Serial.println(String("hot goal:")+goal); 
    }
    else{
      feel[0]=sensors.getTempCByIndex(0);
      if(cold==0)
        cold = sensors.getTempCByIndex(0);
      Serial.println(String("cold goal:")+goal);
    } 
    count++;
    set_temp[count++]=round(((sensors.getTempCByIndex(0)-rt)*10));
    digitalWrite(ledPin, LOW);
    analogWrite(fan_pin,fan_speed);
    if(lower != -1 && upper != -1)
        break;
  }
  for(int i=0;i<6;i+=2){
    Serial.println(set_temp[i]);
    Serial.println(set_temp[i+1]);
    if(set_temp[i]==0)
      break;
    set_table(set_temp[i],set_temp[i+1],q_table);
  }

  if(upper != -1.0 && lower != -1.0){
     goal=(feel[0]+feel[1])/2;
     fan_speed = (upper + lower)/2;
  }
  else if(upper != -1 && lower == -1){
    goal=hot;
     fan_speed = upper;
  }
  else if(upper == -1 && lower != -1){
    goal=hot;
     fan_speed = lower;
  }       
  
  Serial.println(String("temp goal:")+goal); 
  //show_table(q_table);
  //delay(100000);
  send_chart(goal,fan_speed);                                        
  analogWrite(fan_pin,fan_speed);
  //model 1 END
           
  //model 2 START
  while(cnt < episode){                               //train 1000 times
      int action,now_index,action_index;
      float before_tmp,after_tmp;
      sensors.requestTemperatures();//偵測溫度
      before_tmp = sensors.getTempCByIndex(0);
      now_index = check_index(before_tmp,goal);
      //Serial.println(String("before tmp:")+before_tmp);
      //Serial.println(String("now_index:")+now_index);
      if(random(0,100) < 5){        //5%random
        action_index = random(0,20);
            action = (action_index - 10) * 10;
            //Serial.println(String("action index:")+action_index);
            Serial.println(F("random!"));
      }
      else{
        //use q table
          //show_table(q_table);
          action_index = find_max_idx(now_index,q_table);
          action = (action_index - 10) *10;
          //Serial.println(String("action index:")+action_index);
      }
      //Serial.println(String("action:")+action);
      //Serial.println(String("action_index:")+action_index);
      fan_speed += action;
      if(fan_speed>255)
        fan_speed=255;
      else if(fan_speed<0)
        fan_speed=0;
      analogWrite(fan_pin,fan_speed);
      delay(2500);
      
      sensors.requestTemperatures();//偵測溫度
      after_tmp = sensors.getTempCByIndex(0);
      send_chart(after_tmp,fan_speed);
      
      //reward or punish
      int next_index,reward;
      //Serial.println(String("After temp:")+after_tmp);
      //Serial.println(String("now_index:")+now_index);
      next_index = check_index(after_tmp,goal);
      if(after_tmp == goal){
          reward = 20;
      }
      else if(round((after_tmp - before_tmp)*10==0)){
          reward = -20;
      }
      else{
          reward = abs(round((after_tmp - before_tmp)*10));
      }  
      //Serial.println(String("reward:")+reward);
      if(is_good(before_tmp,after_tmp)||round(after_tmp - before_tmp)*10==0){
          //Serial.println("Good");  
          //q_table[now_index][action_index] = round((1-ALPHA)*q_table[now_index][action_index]) + round(ALPHA*(reward + GAMMA*find_max_value(next_index,q_table)));
          q_table[now_index][action_index] += round(ALPHA*(reward + round(GAMMA*find_max_value(next_index,q_table)) - q_table[now_index][action_index] ));
      }
      else{
          //Serial.println(String("now_index:")+now_index);
          //q_table[now_index][action_index] = round((1-ALPHA)*q_table[now_index][action_index]) + round(ALPHA*(-1*reward + GAMMA*find_max_value(next_index,q_table)));
          q_table[now_index][action_index] += round(ALPHA*(-1*reward + round(GAMMA*find_max_value(next_index,q_table)) - q_table[now_index][action_index] ));
      }
      cnt++;
      //Serial.println(String("Q value:")+q_table[now_index][action_index]);
      //Serial.println(cnt);
      
      if(cnt%300==0){
        show_table(q_table);
        update_table(q_table,target);
      }
  }
  //model2 END
}
