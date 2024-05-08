i header.inc
t         <p id="rtc" style="position: relative; text-align: right;" onchange="submit();">
c h          %02d-%02d-20%02d %02d:%02d:%02d</p> <!-- Fecha y hora del RTC-->
t        </td>
t    </div>
t    <div id="paginaPrincipal" class="tabcontent" style="display:block;">
t        <p id="rtcTime" hidden></p>
t        <h1 style="text-align: center;">Real Time Audio Processor</h1>
t        <table class="container">
t            <tr>
t                 <th colspan="4">
t                     <p>Sintonizar una frecuencia (Mhz):</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Seek:</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Volumen:</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Salida:</p>
t                 </th>
t             </tr>
t             <tr>
t                 <td colspan="4" >
t                   <form action="radio.cgi" method="post">
c r 1                 <input type="number" name="frec_sint" step="0.1" max="108" min="87.0"
t                       value="%f" style="text-align: right; width: 80%; font-size: 80%;">
t                     <input type="submit" class="botonsint" value="Sintonizar" name="sintonizar" OnClick="submit();">
t                   </form>
t                 </td>
t                 <td colspan="4" >
t                   <form action="radio.cgi" method="post">
t                     <input type="submit" class="botoncircular" value="&#8679" name="seekup" OnClick="submit();">
t                     <br>
t                     <input type="submit" class="botoncircular" value="&#8681"
t                      name="seekdown" style="margin-top: 5%;" OnClick="submit();">
t                   </form>
t                 </td>
t                 <td colspan="4" >
t                   <form id="form_vol" action="radio.cgi" method="post">
c r 2                 <input type="range" id="vol" name="vol" min="0" max="10" value="%03d"
t                       step="20" onchange="formatValue()">
t                     <input type="hidden" id="formatted_vol" name="formatted_vol">
t                   </form>
t
t                   <form id="form_vol" action="radio.cgi" method="post">
t                     <input type="submit" class="botoncircular" value="&#128263" name="mute"
t                       style="font-family: 'Noto Emoji'" OnClick="submit();">
t                   </form>
t                 </td>
t                 <td colspan="4">
t                   <form action="radio.cgi" method="post">
c r 3                 <input type="radio" id="salida_altavoz" name="salida" value="altavoz" OnClick="submit();" %s>
t                     <label for="salida_altavoz" style="font-size: 20px;">Altavoz</label>
t
c r 4                 <input type="radio" id="salida_cascos" name="salida" value="cascos" OnClick="submit();" %s>
t                     <label for="salida_cascos" style="font-size: 20px; margin-top: 10%;">Cascos</label>
t                   </form>
t                 </td>
t             </tr>
t         </table>
t     </div>
t   <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t   <script>
t   function formatValue() {
t       var vol = document.getElementById("vol").value;
t       var formattedVol = vol.padStart(3, '0');
t       document.getElementById("vol").value = formattedVol;
t       document.getElementById("formatted_vol").value = formattedVol;
t       document.getElementById("form_vol").submit();
t   }
t         var timeUpdate = new periodicObj("time.cgx", 500);
t         function periodicUpdateRTC(){
t           updateMultiple(timeUpdate,  plotRTCTime);
t           rtc_elTime = setTimeout(periodicUpdateRTC, timeUpdate.period);
t         }
t         function plotRTCTime(){
t           timeVal = document.getElementById("rtcTime").value;
t           document.getElementById("rtc").textContent = timeVal;
t         }
t     </script>
t   </body>
t </html>
.