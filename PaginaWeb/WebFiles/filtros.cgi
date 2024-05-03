i header.inc
t         <p id="rtc" style="position: relative; text-align: right;">
c h          %02d-%02d-20%02d %02d:%02d:%02d</p> <!-- Fecha y hora del RTC-->
t        </td>
t    </div>
t    <div id="paginaPrincipal" class="tabcontent" style="display:block;">
t        <p id="rtcTime" hidden></p>
t        <h1 style="text-align: center;">Real Time Audio Processor</h1>
t        <table class="container">
t            <tr>
t             <th colspan="4">
t                 <p>Ecualizador:</p>
t             </th>
t             <th colspan="4">
t                 <p>Guardar Conf:</p>
t             </th>
t             <th colspan="4">
t                 <p>Volumen:</p>
t             </th>
t             <th colspan="4">
t                 <p>Salida:</p>
t             </th>
t         </tr>
t         <tr>
t             <td colspan="4">
t                 <div class="slider-container">
t                   <form action="filtros.cgi" method="post">
t                     <div>
t                     <input type="range" id="eq1" name="eq1" min="-9" max="9"
c f 1                          step="1" value="%d" onchange="submit();">
t                         <label class="slider-label" for="eq1">20Hz</label>
t                     </div>
t                   </form>
t                   <form action="filtros.cgi" method="post">
t                     <div>
t                       <input type="range" id="eq2" name="eq2" min="-9" max="9"
c f 2                       step="1" value="%d" onchange="submit();">
t                        <label class="slider-label" for="eq2">110Hz</label>
t                     </div>
t                   </form>
t                   <form action="filtros.cgi" method="post">
t                     <div>
t                        <input type="range" id="eq3" name="eq3" min="-9" max="9"
c f 3                       value="%d" onchange="submit();">
t                         <label class="slider-label" for="eq3">630Hz</label>
t                     </div>
t                   </form>
t                   <form action="filtros.cgi" method="post">
t                     <div>
t                       <input type="range" id="eq4" name="eq4" min="-9" max="9"
c f 4                        value="%d" onchange="submit();">
t                         <label class="slider-label" for="eq4">3.5kHz</label>
t                     </div>
t                   </form>
t                   <form action="filtros.cgi" method="post">
t                     <div>
t                       <input type="range" id="eq5" name="eq5" min="-9" max="9"
c f 5                       value="%d" onchange="submit();">
t                         <label class="slider-label" for="eq5">20kHz</label>
t                     </div>
t                   </form>
t                 </div>
t             </td>
t
t             <td colspan="4">
t               <form action="filtros.cgi" method="post">
t                 <input type="image" src="imgs/floppy-diskx32.png" name="save_conf" class="botoncircular"
t                 style="font-family: 'Noto Emoji'; margin-bottom: 10%;" OnClick="submit();">
t               </form>
t             </td>
t
t             <td colspan="4" >
t               <form id="form_vol" action="filtros.cgi" method="post">
c f 6             <input type="range" id="vol" name="vol" min="0" max="100" value="%03d"
t                       step="20" onchange="formatValue()">
t                     <input type="hidden" id="formatted_vol" name="formatted_vol">
t                   </form>
t
t                   <form id="form_vol" action="filtros.cgi" method="post">
t                     <input type="submit" class="botoncircular" value="&#128263" name="mute"
t                       style="font-family: 'Noto Emoji'" OnClick="submit();">
t               </form>
t             </td>
t
t             <td colspan="4">
t               <form action="filtros.cgi" method="post">
c f 7             <input type="radio" id="salida_altavoz" name="salida" value="altavoz"OnClick="submit();"  %s>
t                 <label for="salida_altavoz" style="font-size: 20px;">Altavoz</label>
t
c f 8             <input type="radio" id="salida_cascos" name="salida" value="cascos" OnClick="submit();" %s>
t                 <label for="salida_cascos" style="font-size: 20px; margin-top: 10%;">Cascos</label>
t               </form>
t             </td>
t         </tr>
t     </table>
t </div>
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