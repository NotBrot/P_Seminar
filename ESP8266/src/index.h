const char *INDEX PROGMEM = R"""(<!DOCTYPE html>
<html lang="de">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style rel="stylesheet" type="text/css">
      body {
          background-color: #f4f4f4;
          color: #555555;

          font: 16px Arial, sans-serif;
          line-height: 1.6em;

          margin: 0;
      }

      .container{
          width: 80%;
          margin: auto;
          overflow: hidden;
          box-sizing: border-box;
          position: relative;
      }

      header {
          background-color: #414dec;
          color: #fff;

          padding: 12px 20px;
      }

      h1, h2, h3, p {
          margin: unset;
      }

      button {
          background-color: #333;
          color: #fff;

          cursor: pointer;

          padding: 10px 15px;
          border: none;
      }

      button:hover {
          background-color: #ff0000;
      }

      button:active {
          background-color: #ff5c5c;
      }

      .box {
          background-color: #777;
          color: #fff;

          border: 5px #444 solid;
          padding: 20px;
          margin: 20px;
      }

      .value-box {
          background-color: #777;
          /* border: 3px #000 solid; */
          display: table;
          /* padding: 12px 14px; */
          margin: 10px 0px;
      }

      .transfer-indicator {
          height: 20px;
          width: 20px;

          background-color: #00ff00;

          border-radius: 50%;
    
          position: absolute;
          right: 20px;
          top: 20px;
      }

      #messagebox {
          transition: 1s;
          background-color: #da0000;
          color: #fff;

          border: 5px #850000 solid;
          padding: 12px 20px;
          width: 50%;

          position: fixed;
          top: 150px;
          margin: 0 25%;
          box-sizing: border-box;
      }

      #messagebox p {
          margin-bottom: 10px;
      }
    </style>
    <title>Web-Server-Test</title>
  </head>
  <body>
    <header>
      <h1>Web-Server-Test</h1>
    </header>

    <section class="container">
      <div class="box">
        <h2>DHT11</h2>
        <!-- <span id="error-message"> -->
        <div class="values">
          <div class="value-box">
            <p>Temperatur: <span id="temperature"></span>&deg;C</p>
          </div>

          <div class="value-box">
            <p>Luftfeuchtigkeit: <span id="humidity"></span>%</p>
          </div>

          <div class="transfer-indicator"></div>
        </div>

        <button onclick="update()">Update</button>
        <!-- </span> -->
      </div>
    </section>

    <div id="messagebox">
      <p id="message">MessageBox</p>
      <button onclick="document.getElementById('messagebox').hidden = true">
        OK
      </button>
    </div>

    <div style="margin-top: 1000px;"></div>
  </body>
</html>)""";