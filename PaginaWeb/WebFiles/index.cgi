i header.inc
t         <p id="rtc" style="position: relative; text-align: right;" >
c h          %02d-%02d-20%02d %02d:%02d:%02d</p> <!-- Fecha y hora del RTC-->
t        </td>
t    </div>
t    <div id="paginaPrincipal" class="tabcontent" style="display:block;">
t        <p id="rtcTime" hidden></p>
t        <p id="cons_ref"hidden></p>
t        <h1 style="text-align: center;">Real Time Audio Processor</h1>
t        <table class="container">
t            <tr>
t                 <th colspan="3">
t                     <p>Elige camino de audio:</p>
t                 </th>
t                 <th colspan="3">
t                     <p>Bajo Consumo:</p>
t                 </th>
t                 <th colspan="3">
t                     <p>Consumo:</p>
t                 </th>
t             </tr>
t             <tr>
t                 <td colspan="3" >
t                   <form action="index.cgi" method="post">
c i 1                 <input type="radio" id="entrada_radio" name="entrada" value="radio" OnClick="submit();" %s>
t                     <label for="entrada_radio" style="font-size: 20px;">Radio</label>
t
c i 2                 <input type="radio" id="entrada_mp3" name="entrada" value="mp3" OnClick="submit();" %s>
t                     <label for="entrada_mp3" style="font-size: 20px;">MP3</label>
t
t                     <br><br>
c i 3                 <input type="radio" id="salida_altavoz" name="salida" value="altavoz" OnClick="submit();" %s>
t                     <label for="salida_altavoz" style="font-size: 20px;">Altavoz</label>
t
c i 4                 <input type="radio" id="salida_cascos" name="salida" value="cascos" OnClick="submit();" %s>
t                     <label for="salida_cascos" style="font-size: 20px;">Cascos</label>
t                   </form>
t                 </td>
t                 <td colspan="3">
t                   <form action="index.cgi" method="post">
t               	    <input type="submit" class="botoncircular" value="&#128267" name="consumo" OnClick="submit();">
t                   </form>
t                 </td>
t                 <td colspan="3" class="gauge-column">
t                     <div class="gauge-container">
t                         <div id="jg1"></div>
t                     </div>
t                 </td>
t             </tr>
t         </table>
t     </div>
t
t     <script src="https://cdnjs.cloudflare.com/ajax/libs/raphael/2.1.4/raphael-min.js"></script>
t     <script src="https://cdnjs.cloudflare.com/ajax/libs/justgage/1.2.9/justgage.min.js"></script>
t     <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t     <script>
t         var jg1;
t         document.addEventListener("DOMContentLoaded", function (event) {
t             var maxx = 2000;
t
t             jg1 = new JustGage({
t                 id: "jg1",
c i 5             value: %4d,
t                 min: 0,
t                 max: maxx,
t                 title: "Score",
t                 label: "",
t                 pointer: true,
t                 textRenderer: function (val) {
t                     return (val + ' mA' );
t                 },
t                 onAnimationEnd: function () {
t                     // console.log('f: onAnimationEnd()');
t                 }
t             });
t         });
t         var timeUpdate = new periodicObj("time.cgx", 500);
t         function periodicUpdateRTC(){
t           updateMultiple(timeUpdate,  plotRTCTime);
t           rtc_elTime = setTimeout(periodicUpdateRTC, timeUpdate.period);
t         }
t         function plotRTCTime(){
t           timeVal = document.getElementById("rtcTime").value;
t           document.getElementById("rtc").textContent = timeVal;
t           consumo = document.getElementById("cons_ref").value;
t           jg1.refresh(consumo);
t         }
t     </script>
t </body>
t </html>
.