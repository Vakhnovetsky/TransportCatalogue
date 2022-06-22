# TransportCatalogue
* Ввод базы данных и вывод ответа в формате JSON с использованием собственной библиотеки
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
* Стадия make_base: считывание базы из потока ввода в формате JSON и сериализация в бинарный файл. 
* Стадия process_requests: считывание запроса из потока ввода в формате JSON и формирование ответа в поток вывода в формате JSON.\
Запуск производится в консоли с ключами:\
`[make_base|process_requests]`

### JSON файл ввода базы данных стадии make_base
Файл содержит:
* `base_requests` - содержит информацию о маршрутах и остановках
* `render_settings` - настройки для визузализации карты (размер шрифта, толщины линий, цвета и т.д.)
* `routing_settings` - настройки для построения маршрута (время пересадки, скорость движения транспорта)
* `serialization_settings` - содержит имя файла для сериализации
<details>
<summary>Пример файла с базой</summary>
  
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

### JSON файл стадии process_requests
Файл запроса содержит:
* `stat_requests` - содержит запросы типа Bus, Stop, Map, Route
* `serialization_settings` - содержит имя файла для сериализации
<details>
<summary>Пример файла запроса</summary>
  
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
    },
    {
      "id": 749568003,
      "type": "Route",
      "from": "Улица Лизы Чайкиной",
      "to": "Санаторий Родина"
    }
  ]
}
  
```
</details>

### JSON файл стадии process_requests
Файл ответа запрос содержит ответы на запросы типа Bus, Stop, Map, Route с сохранением порядка

<details>
<summary>Пример файла ответов на запрос</summary>
  
```json

[
    {
        "curvature": 1.60481,
        "request_id": 218563507,
        "route_length": 11230,
        "stop_count": 8,
        "unique_stop_count": 7
    },
    {
        "buses": [
            "14",
            "24"
        ],
        "request_id": 508658276
    },
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"125.25,382.708 74.2702,281.925 125.25,382.708\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"592.058,238.297 311.644,93.2643 74.2702,281.925 267.446,450 317.457,442.562 365.599,429.138 367.969,320.138 592.058,238.297\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"367.969,320.138 350.791,243.072 311.644,93.2643 50,50 311.644,93.2643 350.791,243.072 367.969,320.138\" fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgb(255,160,0)\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <circle cx=\"267.446\" cy=\"450\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"317.457\" cy=\"442.562\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"125.25\" cy=\"382.708\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"350.791\" cy=\"243.072\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"365.599\" cy=\"429.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"74.2702\" cy=\"281.925\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"50\" cy=\"50\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"367.969\" cy=\"320.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"592.058\" cy=\"238.297\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"311.644\" cy=\"93.2643\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"black\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"black\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"black\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"black\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"black\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"black\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"black\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n  <text fill=\"black\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n</svg>",
        "request_id": 1359372752
    },
    {
        "items": [
            {
                "stop_name": "Улица Лизы Чайкиной",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "14",
                "span_count": 1,
                "time": 8.6,
                "type": "Bus"
            },
            {
                "stop_name": "Электросети",
                "time": 2,
                "type": "Wait"
            },
            {
                "bus": "24",
                "span_count": 1,
                "time": 9,
                "type": "Bus"
            }
        ],
        "request_id": 749568003,
        "total_time": 21.6
    }
]

```
</details>
