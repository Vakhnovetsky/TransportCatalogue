# TransportCatalogue
* Ввод базы данных и вывод ответа в формате Json с использованием собственной библиотеки
* Визуализация карты маршрутов в формате SVG с использованием собственной библиотеки
* Нахождение самого быстрого маршрута между остановками
* Сериализация и десериализация базы данных с использованием Google Protocol Buffers

![map](https://user-images.githubusercontent.com/88826237/175057806-d675b021-c3a6-4d83-97b9-a6f8d90d4142.png)

## Сборка CMake
1.	Перед сборкой проекта необходимо скачать и собрать Protobuf https://github.com/protocolbuffers/protobuf/releases
2.	Создайте папку для сборки программы. В папку поместите собранные библиотеки Protobuf.
3.	В консоли перейдите в созданный каталог и введите команду:\
`cmake <путь к файлу CMakeLists.txt> -DCMAKE_PREFIX_PATH=<путь к собранной библиотеке Protobuf>`\
По необходимости следует указать ключ с компилятором.
Например: `-G "Visual Studio 17 2022"`
4.	Введите команду: `cmake --build . `
5.	После успешной сборки в каталоге для сборки программы появится выполняемый файл transport_catalogue.exe

## Использование программы
В программе реализована двухстадийность:
* Стадиия make_base: считывание базы из потока ввода в формате Json и сериализация в бинарный файл. 
* Стадиия process_requests: считывание запроса из потока ввода в формате Json и формирование ответа в поток вывода в формате Json
Запуск производится в консоли с ключами:\
`[make_base|process_requests]`

### Json файл ввода базы данных стадии make_base
Файл содержит:
* `base_requests` - содержит информацию о маршрутах и остановках
* `render_settings` - настройки для визузализации карты (размер шрифта, толщины линий, цвета и т.д.)
* `routing_settings` - настройки для построения маршрута (время пересадки, скорость движения транспорта)
* `serialization_settings` - содержит имя файла для сериализации
<details>
<summary>Пример</summary>
  
```json
  
  {
      "serialization_settings": {
          "file": "transport_catalogue.db"
      },
      "routing_settings": {
          "bus_wait_time": 2,
          "bus_velocity": 30
      },
      "render_settings": {
          "width": 1200,
          "height": 500,
          "padding": 50,
          "stop_radius": 5,
          "line_width": 14,
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "stop_label_font_size": 18,
          "stop_label_offset": [
              7,
              -3
          ],
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ]
      },
      "base_requests": [
          {
              "type": "Bus",
              "name": "14",
              "stops": [
                  "Улица Лизы Чайкиной",
                  "Электросети",
                  "Ривьерский мост",
                  "Гостиница Сочи",
                  "Кубанская улица",
                  "По требованию",
                  "Улица Докучаева",
                  "Улица Лизы Чайкиной"
              ],
              "is_roundtrip": true
          },
          {
              "type": "Bus",
              "name": "24",
              "stops": [
                  "Улица Докучаева",
                  "Параллельная улица",
                  "Электросети",
                  "Санаторий Родина"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Bus",
              "name": "114",
              "stops": [
                  "Морской вокзал",
                  "Ривьерский мост"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Stop",
              "name": "Улица Лизы Чайкиной",
              "latitude": 43.590317,
              "longitude": 39.746833,
              "road_distances": {
                  "Электросети": 4300,
                  "Улица Докучаева": 2000
              }
          },
          {
              "type": "Stop",
              "name": "Морской вокзал",
              "latitude": 43.581969,
              "longitude": 39.719848,
              "road_distances": {
                  "Ривьерский мост": 850
              }
          },
          {
              "type": "Stop",
              "name": "Электросети",
              "latitude": 43.598701,
              "longitude": 39.730623,
              "road_distances": {
                  "Санаторий Родина": 4500,
                  "Параллельная улица": 1200,
                  "Ривьерский мост": 1900
              }
          },
          {
              "type": "Stop",
              "name": "Ривьерский мост",
              "latitude": 43.587795,
              "longitude": 39.716901,
              "road_distances": {
                  "Морской вокзал": 850,
                  "Гостиница Сочи": 1740
              }
          },
          {
              "type": "Stop",
              "name": "Гостиница Сочи",
              "latitude": 43.578079,
              "longitude": 39.728068,
              "road_distances": {
                  "Кубанская улица": 320
              }
          },
          {
              "type": "Stop",
              "name": "Кубанская улица",
              "latitude": 43.578509,
              "longitude": 39.730959,
              "road_distances": {
                  "По требованию": 370
              }
          },
          {
              "type": "Stop",
              "name": "По требованию",
              "latitude": 43.579285,
              "longitude": 39.733742,
              "road_distances": {
                  "Улица Докучаева": 600
              }
          },
          {
              "type": "Stop",
              "name": "Улица Докучаева",
              "latitude": 43.585586,
              "longitude": 39.733879,
              "road_distances": {
                  "Параллельная улица": 1100
              }
          },
          {
              "type": "Stop",
              "name": "Параллельная улица",
              "latitude": 43.590041,
              "longitude": 39.732886,
              "road_distances": {}
          },
          {
              "type": "Stop",
              "name": "Санаторий Родина",
              "latitude": 43.601202,
              "longitude": 39.715498,
              "road_distances": {}
          }
      ]
  }
  
```
  
</details>

### Json файл стадии process_requests
Файл запроса содержит:
* `stat_requests` - содержит запросы типа Bus, Stop, Map, Route
* `serialization_settings` - содержит имя файла для сериализации
<details>
<summary>Пример</summary>
  
```json
  
    {
      "serialization_settings": {
          "file": "transport_catalogue.db"
      },
      "stat_requests": [
          {
              "id": 218563507,
              "type": "Bus",
              "name": "14"
          },
          {
              "id": 508658276,
              "type": "Stop",
              "name": "Электросети"
          },
          {
              "id": 1359372752,
              "type": "Map"
          }
		  {
			  "id": 749568003,
			  "type": "Route",
			  "from": "Улица Докучаева",
			  "to": "Санаторий Родина"
    },
      ]
  }
  
```
</details>
