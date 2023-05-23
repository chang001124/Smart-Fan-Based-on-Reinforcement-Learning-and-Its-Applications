#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "WiFiEsp.h"
#define ONE_WIRE_BUS 5 // 1-Wire 總線接口引腳
#define fan_pin 8 // 電風扇 PWM 引腳
#define DHTPIN 9 
#define DHTTYPE DHT11
#define A 21
#define S 151
#define ALPHA 0.1 // Q-learning 學習率
#define GAMMA 0.9 // Q-learning 折扣因子
#define episode 1000
DHT dht(DHTPIN, DHTTYPE);
WiFiEspServer server(80);
RingBuffer buf(8);
char ssid[] = "chang1124";            // your network SSID (name)
char pass[] = "0973365084";        // your network password
const int ledPin = 13;
int repeatCount = 0;
float temperature = 0.0;
int fan_speed = 128;// 電風扇 PWM 引腳
int status = WL_IDLE_STATUS;
float upper = -1.0;
float lower = -1.0; 
float goal = 0.0;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//int q_table[S][A];
int a_table[S];
float rt=23;//室溫
int cnt = 0;
void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);
  WiFi.init(&Serial3);    // initialize ESP module
  sensors.begin();
  dht.begin();  //初始化DHT
  pinMode(fan_pin,OUTPUT);
  pinMode(ledPin, OUTPUT);
  int count=150;
  for(int i=0;i<76;i++){
    a_table[i]=count;
    count-=2;
  }
  for(int i=76;i<S;i++){
    a_table[i]=count;
    count-=2;
  }
  //show_table();
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  // start the web server on port 80
  server.begin();
}
void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  //client.print("Welcome To Smart Fan");
  client.print("Smart Fan");
  
  client.println("<br>");
  client.println("<br>");
  
  client.println("Click<button> <a href=\"/H\">cold</a> </button>feel cold<br>");
  client.println("Click<button> <a href=\"/L\">hot</a> </button>feel hot<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
void set_table(int number,int a,int q_table[][A]){//
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
      q_table[i][j]+=100;
  }
  else{
    for(int j=6;j>=0;j--)
      q_table[i][j]+=100;
  }
}
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
int find_max_idx(int row,int q_table[S][A]){
    int Max = -1,i,idx;
    for(i=0;i<A;i++){
      if(q_table[row][i]>Max){
        Max = q_table[row][i];
        idx=i;
      }
    }
    if(Max == 0){
        idx = random(0,20);
    }
    
    return idx; 
}
bool is_good(float before,float after){
    if(abs(before - goal)>abs(after - goal)){
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
void loop() {
  rt = dht.readTemperature();  //取得溫度C
  Serial.println(String("rt=")+rt);
  int q_table[S][A],set_temp[6],flag=0;
  for(int i=0;i<S;i++){
    for(int j=0;j<A;j++){
      q_table[i][j]=0;
    }
  }
  for(int i=0;i<6;i++)
    set_temp[i]=0;
  //show_table(q_table);
  analogWrite(fan_pin,fan_speed);
  //model 1 START
  int count=0;
  while(repeatCount!=3){
    flag=0;
    WiFiEspClient client = server.available();  // listen for incoming clients
    if (client) {                               // if you get a client,
      Serial.println("New client");             // print a message out the serial port
      buf.init();                               // initialize the circular buffer
      while (client.connected()) {              // loop while the client's connected
        if (client.available()) {               // if there's bytes to read from the client,
          char c = client.read();               // read a byte, then
          buf.push(c);                          // push it to the ring buffer
          if (buf.endsWith("\r\n\r\n")) {
            sendHttpResponse(client);
            break;
          }
          
          // Check to see if the client request was "GET /H" or "GET /L":
          if (buf.endsWith("GET /H")) {
            fan_speed -= 40;
            set_temp[count++]=-40;
            lower = fan_speed;
            flag=1;
            digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
          }
          else if (buf.endsWith("GET /L")) {
            fan_speed += 40;
            set_temp[count++]=40;
            upper = fan_speed;
            flag=1;
            digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
          }
        }
      }
      if(flag==1){
        sensors.requestTemperatures();//偵測溫度
        set_temp[count++]=round(((sensors.getTempCByIndex(0)-rt)*10));
        repeatCount++;
      }
      analogWrite(fan_pin,fan_speed);
      if(lower != -1 && upper != -1)
        repeatCount=3;
      client.stop();
      Serial.println("Client disconnected");
      Serial.println(" ");
    }
    
  }
  for(int i=0;i<6;i+=2){
    Serial.println(set_temp[i]);
    Serial.println(set_temp[i+1]);
    if(set_temp[i]==0)
      break;
    set_table(set_temp[i],set_temp[i+1],q_table);
  }
  show_table(q_table);
  sensors.requestTemperatures();//偵測溫度
  goal = sensors.getTempCByIndex(0);
  Serial.println(String("temp goal:")+goal);
  if(upper != -1.0 && lower != -1.0){
     fan_speed = (upper + lower)/2;
  }
  else if(upper != -1 && lower == -1){
     fan_speed = upper;
  }
  else if(upper == -1 && lower != -1){
     fan_speed = lower;
  }                                                
  analogWrite(fan_pin,fan_speed);
  //Serial.println(String("fan_speed:")+fan_speed);     //model 1 END
           
  //model 2 START
  while(cnt < episode){                               //train 1000 times
      int action,now_index,action_index;
      float before_tmp,after_tmp;
      sensors.requestTemperatures();//偵測溫度
      before_tmp = sensors.getTempCByIndex(0);
      now_index = check_index(before_tmp,goal);
      Serial.println(String("before tmp:")+before_tmp);
      //Serial.println(String("now_index:")+now_index);
      if(random(0,100) < 5){        //5%random
        action_index = random(0,20);
            action = (action_index - 10) * 10;
      }
      else{                         //use q table
          action_index = find_max_idx(now_index,q_table);
          action = (action_index - 10) * 10;
      }
      Serial.println(String("action:")+action);
      //Serial.println(String("action_index:")+action_index);
      fan_speed += action;
      analogWrite(fan_pin,fan_speed);
      delay(500);
      
      sensors.requestTemperatures();//偵測溫度
      after_tmp = sensors.getTempCByIndex(0);

      
      //reward or punish
      int next_index,reward;
      Serial.println(String("After temp:")+after_tmp);
      //Serial.println(String("now_index:")+now_index);
      next_index = check_index(after_tmp,goal);
      if(after_tmp == goal){
          reward = 10;
      }
      else if(round(after_tmp - before_tmp)==0){
          reward = -10;
      }
      else{
          reward = abs(round((after_tmp - before_tmp)*10));
      }  
      
      if(is_good(before_tmp,after_tmp)){  
          q_table[now_index][action_index] = 10*((1-ALPHA)*q_table[now_index][action_index] + ALPHA*(reward + GAMMA*find_max_value(next_index,q_table)));
      }
      else{
          //Serial.println(String("now_index:")+now_index);
          q_table[now_index][action_index] = 10*((1-ALPHA)*q_table[now_index][action_index] + ALPHA*(-reward + GAMMA*find_max_value(next_index,q_table)));
          
      }
      cnt++;
      //Serial.println(String("Q value:")+q_table[now_index][action_index]);
      Serial.println(cnt);
      //if(cnt%10==0)
        show_table(q_table);
  }
  //model2 END
}
