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
t                     <p>Canciones:</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Control:</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Volumen:</p>
t                 </th>
t                 <th colspan="4">
t                     <p>Salida:</p>
t                 </th>
t             </tr>
t             <tr>
t                 <td colspan="4" style="position: relative;">
t                     <form action="mp3.cgi" method="post">
t                     <select name="canciones" id="listacaniones" style="width: 90%; height: 50px;">
t                         <option value="default">Selecciona una canción</option>
c m a                     <option value="01">%s</option>
c m b                     <option value="02">%s</option>
c m c                     <option value="03">%s</option>
c m d                     <option value="04">%s</option>
c m e                     <option value="05">%s</option>
c m f                     <option value="06">%s</option>
c m g                     <option value="07">%s</option>
c m h                     <option value="08">%s</option>
c m i                     <option value="09">%s</option>
c m j                     <option value="10">%s</option>
c m k                     <option value="11">%s</option>
c m l                     <option value="12">%s</option>
c m m                     <option value="13">%s</option>
c m n                     <option value="14">%s</option>
c m o                     <option value="15">%s</option>
c m p                     <option value="16">%s</option>
c m q                     <option value="17">%s</option>
c m r                     <option value="18">%s</option>
c m s                     <option value="19">%s</option>
c m t                     <option value="20">%s</option>
c m u                     <option value="21">%s</option>
c m v                     <option value="22">%s</option>
c m w                     <option value="23">%s</option>
c m x                     <option value="24">%s</option>
c m y                     <option value="25">%s</option>
t                     </select>
t                       <input type="image" src="imgs/sendx32.png" class="botoncircular"
t                         style="position: absolute; right: 0; top: 50%; transform: translateY(-50%);
t                         margin-left: 0px; width: 40px; height: 40px;"
t                         name="send_song" OnClick="submit();">
t                     </form>
t                 </td>
t
t
t                 <td colspan="4">
t                     <div class="control-container">
t                       <form action="mp3.cgi" method="post">
t                         <input type="image" src="imgs/prev-songx32.png" class="botoncircular"
t                          style="margin-right: 5px; width: 40px; height: 40px;" name="prev_song" OnClick="submit();">
t
t                         <input type="image" src="imgs/play-pausex32.png" class="botoncircular"
t                          style="margin-right: 5px; width: 40px; height: 40px;"name="play_song" OnClick="submit();">
t
t                         <input type="image" src="imgs/next-songx32.png" class="botoncircular"
t                          style="margin-right: 5px; width: 40px; height: 40px;"name="next_song" OnClick="submit();">
t
t                         <input type="image" src="imgs/loopx32.png" class="botoncircular"
t                          style="margin-right: 5px; width: 40px; height: 40px;"name="loop_song" OnClick="submit();">
t
t                       </form>
t                     </div>
t                 </td>
t                 <td colspan="4">
t                   <form id="form_vol" action="mp3.cgi" method="post">
c m 6                 <input type="range" id="vol" name="vol" min="0" max="10" value="%03d"
t                       step="20" onchange="formatValue()">
t                     <input type="hidden" id="formatted_vol" name="formatted_vol">
t                   </form>
t
t                   <form id="form_vol" action="mp3.cgi" method="post">
t                     <input type="submit" class="botoncircular" value="&#128263" name="mute"
t                       style="font-family: 'Noto Emoji'" OnClick="submit();">
t                   </form>
t                 </td>
t                 <td colspan="4">
t                 <form action="mp3.cgi" method="post">
c m 7                 <input type="radio" id="salida_altavoz" name="salida" value="altavoz" OnClick="submit();" %s>
t                     <label for="salida_altavoz" style="font-size: 20px;">Altavoz</label>
t
c m 8                 <input type="radio" id="salida_cascos" name="salida" value="cascos" OnClick="submit();" %s>
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