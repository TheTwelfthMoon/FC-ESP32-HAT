#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ARA_ESP.h"

const char* ssid = "EducationalDrone";//Имя точки доступа
const char* password = "12345678";//Пароль от точки доступа

AsyncWebServer server(8775);//порт подключения
AsyncWebSocket ws("/ws");// Вебсокет

IPAddress local_ip(192, 168, 2, 113);// Локальный IP-адрес
IPAddress gateway(192, 168, 2, 113);// IP-адрес шлюза
IPAddress subnet(255, 255, 255, 0);// Маска подсети

//глобальные переменные для хранения значений крена, тангажа, рысканья и дросселя, которые будут получены через веб-сокет.
float RollValue = 0;
float PitchValue = 0;
float YawValue = 0;
float ThrottleValue = 0;

//Эта функция разделяет входную строку на массив значений с плавающей запятой с использованием указанного разделителя. 
//Разделенные значения преобразуются в числа с плавающей запятой и сохраняются в предоставленном массиве.
void splitAndConvertToFloat(const String &inputString, float array[], int arraySize, char delimiter) {
  char temp[inputString.length() + 1];
  inputString.toCharArray(temp, inputString.length() + 1);

  char *token = strtok(temp, ";");
  int count = 0;

  while (token != NULL && count < arraySize) {
    array[count] = atof(token);
    token = strtok(NULL, ";");
    count++;
  }
}
//Эта функция обрабатывает входящие данные веб-сокета. Она извлекает данные в виде строки и разделяет их на массив значений с плавающей запятой с использованием точки с запятой в качестве разделителя. 
//Затем разделенные значения присваиваются соответствующим глобальным переменным YawValue, ThrottleValue, RollValue и PitchValue.
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    data[len] = 0; // Ensure null-terminated string

    //Serial.printf("Data received: %s\n", (char*)data);
    String inputString = String((char*) data);
    float values[6];

    splitAndConvertToFloat(inputString, values, 6, ';');

    YawValue = values[2];
    ThrottleValue = values[3];
    RollValue = values[4];
    PitchValue = values[5];
  
  }
}
//код html-страницы
const char* html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Panel</title>
    <style>
        /* Disable text selection */
        body {
            -webkit-user-select: none; /* Safari */
            -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* IE10+/Edge */
            user-select: none; /* Standard */
            touch-action: none;
            -ms-content-zooming: none;            
        }

        /* Style for the control panel */
        #control-panel {
            width: 600px;
            height: 200px;
            margin: 0 auto;
            padding: 20px;
            border: 1px solid #ccc;
            border-radius: 20px;
            background-color: #f9f9f9;
        }

        /* Style for the joysticks */
        .joystick {
            width: 250px;
            height: 150px;
            background-color: #ddd;
            border-radius: 25px;
            display: inline-block;
            margin-right: 20px;
            position: relative;
            overflow: hidden;
            
        }

        .joystick-handle {
            width: 50px;
            height: 50px;
            background-color: #4cafa8;
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }

        input[name=toggleSwitch] {
            height: 30px;
            width: 30px;
            appearance: none;
            background-color: #4cafa8;
            border-radius: 50%;
            cursor: pointer;
            opacity: 0;
            margin: 0;
        }

        .tri-state-toggle {
            width: fit-content;
            display: flex;
            justify-content: center;
            border: 3px solid #ccc;
            border-radius: 50px;
        }

        .joystick-container {
            width: 700px;
            height: 300px;
            /* display: flex; */
            flex-direction: row;
            justify-content: space-evenly;
        }

        .toggles-container {
            margin: 20px;
            display: flex;
            flex-direction: row;
            justify-content: space-evenly;
        }

    </style>
</head>
<body>
<div id="control-panel">
    <div class="joystick-container">
        <div class="joystick" id="left-joystick">
            <div class="joystick-handle" id="left-handle"></div>
        </div>
        <div style="right: -11%;" class="joystick" id="right-joystick">
            <div class="joystick-handle" id="right-handle"></div>
        </div>
    </div>
    <!-- <div class="toggles-container">
        <div class="tri-state-toggle">
            <input class="button" type="radio" name="toggleSwitch" id="one" data-id="0"/>
            <input class="button" type="radio" name="toggleSwitch" id="two" data-id="1"/>
            <input class="button" type="radio" name="toggleSwitch" id="three" data-id="2"/>
        </div>
    </div> -->
</div>

<script>
    // JOYSTICKS
    const leftHandle = document.getElementById('left-handle');
    const rightHandle = document.getElementById('right-handle');
    const leftJoystick = document.getElementById('left-joystick');
    const rightJoystick = document.getElementById('right-joystick');

    let allReadings = [0, 1, 0, 0, 0, 0] // [armed, state, LX, LY, RX, RY]

    function updateJoystickPosition(mouseX, mouseY, joystick, handle, shift, initialPositionY) {
        const handleMargin = 10;
        const joystickRect = joystick.getBoundingClientRect();

        const centerX = (joystickRect.width / 2) - 25;
        const centerY = (joystickRect.height / 2) + 25 * initialPositionY;

        let currentY = Math.min(Math.max(mouseY - 25 - joystickRect.top, 0), joystickRect.height - 50)
        let currentX = Math.min(Math.max(mouseX - 25 - joystickRect.left, 0), joystickRect.width - 50)

        let actualReadingX = currentX / centerX - 1;
        let actualReadingY = 1 - currentY / centerY;

        handle.style.left = `${currentX + 25}px`;
        handle.style.top = `${currentY + 25}px`;

        allReadings[shift] = actualReadingX;
        allReadings[shift+1] = actualReadingY;
        console.log(allReadings);
    }

    function hydrateJoystickLeft() {
        const shift = 2;
        leftHandle.style.left = "50%";
        leftHandle.style.top = `calc(100% - 25px)`;

        // s.value = shift;

        leftJoystick.addEventListener('mousedown', function (event) {
            updateJoystickPosition(event.clientX, event.clientY, leftJoystick, leftHandle, shift, 1);
            const handleMouseMove = function (event) {
                updateJoystickPosition(event.clientX, event.clientY, leftJoystick, leftHandle, shift, 1);
            };
            document.addEventListener('mousemove', handleMouseMove);
            document.addEventListener('mouseup', function () {
                allReadings[shift] = 0;
                document.removeEventListener('mousemove', handleMouseMove);
                leftHandle.style.left = "50%";
            }, {once: true});
        });

        leftJoystick.addEventListener('touchstart', function (event) {
            var indexL = event.touches.length - 1;
            updateJoystickPosition(event.touches[indexL].clientX, event.touches[indexL].clientY, leftJoystick, leftHandle, shift, 1);
            const handleMouseMove = function (event) {
                updateJoystickPosition(event.touches[indexL].clientX, event.touches[indexL].clientY, leftJoystick, leftHandle, shift, 1);
            };
            leftJoystick.addEventListener('touchmove', handleMouseMove);
            leftJoystick.addEventListener('touchend', function () {
                allReadings[shift] = 0;
                // document.removeEventListener('touchmove', handleMouseMove);
                leftHandle.style.left = "50%";
            }, {once: true});
        });
    }

    function hydrateJoystickRight() {
        const shift = 4;
        rightHandle.style.left = "50%";
        rightHandle.style.top = "50%";

        rightJoystick.addEventListener('mousedown', function (event) {
            updateJoystickPosition(event.clientX, event.clientY, rightJoystick, rightHandle, shift, -1);
            const handleMouseMove = function (event) {
                updateJoystickPosition(event.clientX, event.clientY, rightJoystick, rightHandle, shift, -1);
            };
            document.addEventListener('mousemove', handleMouseMove);
            document.addEventListener('mouseup', function () {
                allReadings[shift] = 0
                allReadings[shift+1] = 0
                document.removeEventListener('mousemove', handleMouseMove);
                rightHandle.style.left = "50%";
                rightHandle.style.top = "50%";
            }, {once: true});
        });

        rightJoystick.addEventListener('touchstart', function (event) {
            var indexR = event.touches.length - 1;
            updateJoystickPosition(event.touches[indexR].clientX, event.touches[indexR].clientY, rightJoystick, rightHandle, shift, -1);
            const handleMouseMove = function (event) {
                updateJoystickPosition(event.touches[indexR].clientX, event.touches[indexR].clientY, rightJoystick, rightHandle, shift, -1);
            };
            rightJoystick.addEventListener('touchmove', handleMouseMove);
            rightJoystick.addEventListener('touchend', function () {
                allReadings[shift] = 0
                allReadings[shift+1] = 0
                // rightJoystick.removeEventListener('touchmove', handleMouseMove);
                rightHandle.style.left = "50%";
                rightHandle.style.top = "50%";
            }, {once: true});
        });
    }

    hydrateJoystickLeft()
    hydrateJoystickRight()
    window.addEventListener('scroll-fix', function(e) {e.preventDefault();});

    // TOGGLES_LEFT
    // let toggleState = 1;
    // const toggleSwitchStates = document.querySelectorAll('[name=toggleSwitch]')

    // toggleSwitchStates.forEach((el) => {
    //     if (+el.dataset.id === toggleState) el.style.opacity = "1"
    //     el.addEventListener("click", (ev) => {
    //         toggleSwitchStates.forEach((elem) => elem.style.opacity = "0")
    //         el.style.opacity = "1"
    //         allReadings[1] = +el.dataset.id
    //     })
    // })

    // NETWORK
    websocket_url = "ws://192.168.2.113:8775/ws";
    sendTimeout = 100;
    const websocket = new WebSocket(websocket_url);

    const sendLoop = setInterval(() => {
        if (websocket.readyState === websocket.OPEN)
            websocket.send(allReadings.join(';'));
        else {
            console.log(websocket.readyState);
            console.log("Socket not working!!!");
            clearInterval(sendLoop);
        }
    }, sendTimeout)
</script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);// Инициализировать последовательный порт

  WiFi.softAP(ssid, password);// Создать точку доступа с именем ssid и паролем password
  WiFi.softAPConfig(local_ip, gateway, subnet);// Настроить конфигурацию точки доступа (локальный IP, шлюз, маска подсети)
  
  Serial.print("AP IP Address: ");// Вывести IP-адрес точки доступа в последовательный порт
  Serial.println(WiFi.softAPIP());

  ws.onEvent(onWsEvent);// Настроить веб-сокет
  server.addHandler(&ws);// Добавить обработчик веб-сокета на сервер

 // Обработчик запросов GET по адресу "/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html);
  });

  server.begin();// Запустить сервер
  esp.begin(Serial);// Запустить сервис по отправки данных в полетный контроллер
}


void loop() {
  esp.roll(RollValue);// Установить крен
  delay(20);//50

  esp.pitch(PitchValue);// Установить тангаж
  delay(20);

  esp.yaw(YawValue);// Установить рысканье
  delay(20);

  esp.throttle(ThrottleValue);// Установить дроссель
  delay(20);

}
