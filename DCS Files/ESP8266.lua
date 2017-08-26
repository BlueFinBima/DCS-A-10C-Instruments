--***DO NOT MODIFY THESE COMMENT LINES***
-- This file is for the interaction with the ESP8266 panels and lights infrastructure which is running on a NodeMCU
-- and listening on port 3339
--
--NL DCS Tweaks
--LOCATION NL_Mods
--HOOKTO !USERPROFILE!\Saved Games\DCS\Scripts\Export.lua
--Export lua

-- Prev Export functions to allow us to hook into the chain of routines.
print("NL Mods - ESP8266.lua Invoked.\n")

local PrevExport = {}
PrevExport.LuaExportStart = LuaExportStart
PrevExport.LuaExportStop = LuaExportStop
PrevExport.LuaExportBeforeNextFrame = LuaExportBeforeNextFrame
PrevExport.LuaExportAfterNextFrame = LuaExportAfterNextFrame
PrevExport.LuaExportActivityNextEvent = LuaExportActivityNextEvent

gESP8266host = "10.1.1.17"
gESP8266Port = 3339
gESP8266ExportInterval = 0.067
gESP8266ExportLowTickInterval = 1
--
-- info for this is in ??\DCS World\Mods\aircraft\A-10C\Cockpit\Scripts\mainpanel_init.lua
-- 
gESP8266EveryFrameArguments = {
    [13]="%.4f",
    [14]="%.4f",
    [372]="%0.1f",
    [373]="%0.1f",
    [374]="%0.1f",
    [404]="%0.1f",
    [480]="%0.1f",
    [481]="%0.1f",
    [482]="%0.1f",
    [483]="%0.1f",
    [484]="%0.1f",
    [485]="%0.1f",
    [486]="%0.1f",
    [487]="%0.1f",
    [488]="%0.1f",
    [489]="%0.1f",
    [490]="%0.1f",
    [491]="%0.1f",
    [492]="%0.1f",
    [493]="%0.1f",
    [494]="%0.1f",
    [495]="%0.1f",
    [496]="%0.1f",
    [497]="%0.1f",
    [498]="%0.1f",
    [499]="%0.1f",
    [500]="%0.1f",
    [501]="%0.1f",
    [502]="%0.1f",
    [503]="%0.1f",
    [504]="%0.1f",
    [505]="%0.1f",
    [506]="%0.1f",
    [507]="%0.1f",
    [508]="%0.1f",
    [509]="%0.1f",
    [510]="%0.1f",
    [511]="%0.1f",
    [512]="%0.1f",
    [513]="%0.1f",
    [514]="%0.1f",
    [515]="%0.1f",
    [516]="%0.1f",
    [517]="%0.1f",
    [518]="%0.1f",
    [519]="%0.1f",
    [520]="%0.1f",
    [521]="%0.1f",
    [522]="%0.1f",
    [523]="%0.1f",
    [524]="%0.1f",
    [525]="%0.1f",
    [526]="%0.1f",
    [527]="%0.1f",
    [540]="%0.1f",
    [541]="%0.1f",
    [542]="%0.1f",
    [606]="%0.1f",
    [608]="%0.1f",
    [610]="%0.1f",
    [612]="%0.1f",
    [614]="%0.1f",
    [616]="%0.1f",
    [618]="%0.1f",
    [619]="%0.1f",
    [620]="%0.1f",
    [659]="%0.1f",
    [660]="%0.1f",
    [661]="%0.1f",
    [662]="%0.1f",
    [663]="%0.1f",
    [664]="%0.1f",
    [665]="%0.1f",
    [730]="%0.1f",
    [731]="%0.1f",
    [732]="%0.1f",
    [737]="%0.1f"
}

gESP8266Arguments = {
    [22]="%.3f",
    [101]="%.1f",
    [102]="%1d",
    [103]="%1d",
    [104]="%1d",
    [105]="%1d",
    [106]="%1d",
    [107]="%1d",
    [108]="%1d",
    [109]="%1d",
    [110]="%1d",
    [111]="%1d",
    [112]="%1d",
    [113]="%1d",
    [114]="%1d",
    [115]="%.1f",
    [116]="%.3f",
    [117]="%1d",
    [118]="%1d",
    [119]="%1d",
    [120]="%1d",
    [121]="%1d",
    [122]="%1d",
    [123]="%1d",
    [124]="%1d",
    [125]="%1d",
    [126]="%1d",
    [127]="%.1f",
    [130]="%1d",
    [131]="%.1f",
    [132]="%1d",
    [133]="%.3f",
    [134]="%1d",
    [135]="%0.1f",
    [136]="%.1f",
    [137]="%0.3f",
    [138]="%0.1f",
    [139]="%0.2f",
    [140]="%0.2f",
    [141]="%0.2f",
    [142]="%0.2f",
    [147]="%.3f",
    [148]="%1d",
    [149]="%0.1f",
    [150]="%.1f",
    [151]="%0.3f",
    [152]="%0.1f",
    [153]="%0.2f",
    [154]="%0.2f",
    [155]="%0.2f",
    [156]="%0.2f",
    [161]="%0.2f",
    [162]="%0.1f",
    [163]="%0.2f",
    [164]="%0.2f",
    [165]="%0.2f",
    [166]="%0.2f",
    [167]="%0.1f",
    [168]="%0.1f",
    [169]="%1d",
    [170]="%1d",
    [171]="%.3f",
    [172]="%.1f",
    [173]="%.1f",
    [174]="%1d",
    [175]="%1d",
    [176]="%0.1f",
    [177]="%1d",
    [180]="%1d",
    [183]="%1d",
    [184]="%1d",
    [189]="%1d",
    [190]="%.1f",
    [192]="%.3f",
    [193]="%.3f",
    [194]="%0.1f",
    [195]="%.3f",
    [196]="%1d",
    [197]="%.1f",
    [198]="%.1f",
    [199]="%0.1f",
    [200]="%0.1f",
    [201]="%1d",
    [202]="%1d",
    [203]="%1d",
    [204]="%1d",
    [205]="%1d",
    [206]="%1d",
    [207]="%1d",
    [208]="%1d",
    [209]="%0.2f",
    [210]="%0.2f",
    [211]="%0.2f",
    [212]="%0.2f",
    [213]="%0.2f",
    [214]="%0.2f",
    [221]="%.3f",
    [222]="%1d",
    [223]="%.3f",
    [224]="%1d",
    [225]="%.3f",
    [226]="%1d",
    [227]="%.3f",
    [228]="%1d",
    [229]="%.3f",
    [230]="%1d",
    [231]="%.3f",
    [232]="%1d",
    [233]="%.3f",
    [234]="%1d",
    [235]="%.3f",
    [236]="%1d",
    [237]="%1d",
    [238]="%.3f",
    [239]="%0.1f",
    [240]="%.1f",
    [241]="%1d",
    [242]="%1d",
    [243]="%1d",
    [244]="%1d",
    [245]="%1d",
    [246]="%1d",
    [247]="%1d",
    [248]="%0.1f",
    [249]="%.3f",
    [250]="%0.1f",
    [251]="%0.1f",
    [252]="%0.1f",
    [258]="%0.2f",
    [259]="%.1f",
    [261]="%.3f",
    [262]="%0.1f",
    [266]="%1d",
    [267]="%.1f",
    [268]="%.3f",
    [270]="%1d",
    [271]="%.3f",
    [272]="%1d",
    [273]="%1d",
    [275]="%.1f",
    [276]="%1d",
    [277]="%.3f",
    [278]="%1d",
    [279]="%1d",
    [280]="%1d",
    [282]="%1d",
    [283]="%1d",
    [284]="%.3f",
    [287]="%1d",
    [288]="%.3f",
    [290]="%.3f",
    [291]="%1d",
    [292]="%.3f",
    [293]="%.3f",
    [294]="%1d",
    [295]="%1d",
    [296]="%.3f",
    [297]="%.3f",
    [300]="%.1f",
    [301]="%.1f",
    [302]="%.1f",
    [303]="%.1f",
    [304]="%.1f",
    [305]="%.1f",
    [306]="%.1f",
    [307]="%.1f",
    [308]="%.1f",
    [309]="%.1f",
    [310]="%.1f",
    [311]="%.1f",
    [312]="%.1f",
    [313]="%.1f",
    [314]="%.1f",
    [315]="%.1f",
    [316]="%.1f",
    [317]="%.1f",
    [318]="%.1f",
    [319]="%.1f",
    [320]="%1d",
    [321]="%1d",
    [322]="%1d",
    [323]="%1d",
    [324]="%1d",
    [325]="%0.1f",
    [326]="%.1f",
    [327]="%.1f",
    [328]="%.1f",
    [329]="%.1f",
    [330]="%.1f",
    [331]="%.1f",
    [332]="%.1f",
    [333]="%.1f",
    [334]="%.1f",
    [335]="%.1f",
    [336]="%.1f",
    [337]="%.1f",
    [338]="%.1f",
    [339]="%.1f",
    [340]="%.1f",
    [341]="%.1f",
    [342]="%.1f",
    [343]="%.1f",
    [344]="%.1f",
    [345]="%.1f",
    [346]="%1d",
    [347]="%1d",
    [348]="%1d",
    [349]="%1d",
    [350]="%1d",
    [351]="%0.1f",
    [352]="%.1f",
    [353]="%.1f",
    [354]="%.1f",
    [355]="%.1f",
    [356]="%1d",
    [357]="%.1f",
    [358]="%1d",
    [359]="%.3f",
    [360]="%0.1f",
    [361]="%0.1f",
    [362]="%0.1f",
    [363]="%0.1f",
    [364]="%0.1f",
    [365]="%.1f",
    [366]="%.1f",
    [367]="%.3f",
    [368]="%.3f",
    [369]="%.1f",
    [370]="%.1f",
    [371]="%.1f",
    [375]="%0.1f",
    [376]="%0.1f",
    [377]="%0.1f",
    [378]="%1d",
    [379]="%0.1f",
    [380]="%1d",
    [381]="%1d",
    [382]="%1d",
    [383]="%1d",
    [384]="%0.1f",
    [385]="%.1f",
    [386]="%.1f",
    [387]="%.1f",
    [388]="%.1f",
    [389]="%.1f",
    [390]="%.1f",
    [391]="%.1f",
    [392]="%.1f",
    [393]="%.1f",
    [394]="%.1f",
    [395]="%.1f",
    [396]="%.1f",
    [397]="%.1f",
    [398]="%.1f",
    [399]="%.1f",
    [400]="%.1f",
    [401]="%.1f",
    [402]="%.1f",
    [403]="%.1f",
    [405]="%1d",
    [406]="%1d",
    [407]="%1d",
    [408]="%1d",
    [409]="%1d",
    [410]="%.1f",
    [411]="%.1f",
    [412]="%.1f",
    [413]="%.1f",
    [414]="%.1f",
    [415]="%.1f",
    [416]="%.1f",
    [417]="%.1f",
    [418]="%.1f",
    [419]="%.1f",
    [420]="%.1f",
    [421]="%.1f",
    [422]="%.1f",
    [423]="%.1f",
    [424]="%1d",
    [425]="%.1f",
    [426]="%.1f",
    [427]="%.1f",
    [428]="%.1f",
    [429]="%.1f",
    [430]="%.1f",
    [431]="%.1f",
    [432]="%.1f",
    [433]="%.1f",
    [434]="%.1f",
    [435]="%.1f",
    [436]="%.1f",
    [437]="%.1f",
    [438]="%.1f",
    [439]="%.1f",
    [440]="%.1f",
    [441]="%.1f",
    [442]="%.1f",
    [443]="%.1f",
    [444]="%.1f",
    [445]="%.1f",
    [446]="%.1f",
    [447]="%.1f",
    [448]="%.1f",
    [449]="%.1f",
    [450]="%.1f",
    [451]="%.1f",
    [452]="%.1f",
    [453]="%.1f",
    [454]="%.1f",
    [455]="%.1f",
    [456]="%.1f",
    [457]="%.1f",
    [458]="%.1f",
    [459]="%.1f",
    [460]="%.1f",
    [461]="%.1f",
    [462]="%.1f",
    [463]="%1d",
    [466]="%.1f",
    [467]="%.1f",
    [468]="%.1f",
    [469]="%1d",
    [470]="%.1f",
    [471]="%.1f",
    [472]="%1d",
    [473]="%0.1f",
    [474]="%1d",
    [475]="%0.1f",
    [476]="%1d",
    [477]="%1d",
    [531]="%.1f",
    [532]="%.1f",
    [533]="%.1f",
    [601]="%1d",
    [602]="%1d",
    [603]="%1d",
    [605]="%.1f",
    [607]="%.1f",
    [609]="%.1f",
    [611]="%.1f",
    [613]="%.1f",
    [615]="%.1f",
    [617]="%.1f",
    [621]="%1d",
    [622]="%0.1f",
    [623]="%1d",
    [624]="%.3f",
    [626]="%.3f",
    [628]="%.1f",
    [630]="%.1f",
    [632]="%.1f",
    [634]="%.1f",
    [636]="%0.2f",
    [638]="%0.2f",
    [640]="%0.2f",
    [642]="%0.2f",
    [644]="%1d",
    [645]="%0.1f",
    [646]="%.1f",
    [651]="%.1f",
    [655]="%0.1f",
    [704]="%.3f",
    [705]="%.3f",
    [711]="%.1f",
    [712]="%0.2f",
    [716]="%1d",
    [718]="%1d",
    [722]="%.1f",
    [733]="%1d",
    [734]="%1d",
    [735]="%.1f",
    [772]="%1d",
    [778]="%1d",
    [779]="%1d",
    [780]="%1d",
    [781]="%0.1f",
    [782]="%0.1f",
    [783]="%0.1f",
    [784]="%1d"
}


gEnableDebuglogFileESP8266 = true

local lfs = require('lfs')
local lDebugLogFileName = lfs.writedir().."Logs\\NL_ESP8266.log"
local function WriteToLogESP8266(message)
    if gEnableDebuglogFileESP8266 then
    	if logFileESP8266 then
    		logFileESP8266:write(string.format("%s: %s\r\n", os.date("%H:%M:%S"), message))
        else
            print(string.format("%s: %s\r\n", os.date("%H:%M:%S"), message))
        end
    end
end

if gEnableDebuglogFileESP8266 then
    logFileESP8266 = assert(io.open(lDebugLogFileName, "w"))
    if logFileESP8266 then
           WriteToLogESP8266(string.format("Logfile is now open.\n"))
           print(string.format("Logfile opened:" .. lDebugLogFileName .. "\n"))
    else
           print(string.format("Logfile did not open!" .. lDebugLogFileName .. "\n"))
    end
end

function ProcessHighImportanceESP8266(mainPanelDevice)
    -- This is the text for the CMSP Counter Measures
    SendDataESP8266(2087, list_indication(7))
    -- This is the text for the Front Counter Measures
    SendDataESP8266(2088, list_indication(8))
end
function ProcessLowImportanceESP8266(mainPanelDevice)
    WriteToLogESP8266("Entered ProcessLowImportanceLights")
	-- Get Radio Frequencies
	local lUHFRadio = GetDevice(54)
	SendDataESP8266(2000, string.format("%7.3f", lUHFRadio:get_frequency()/1000000))
	-- ILS Frequency
	--SendDataESP8266(2251, string.format("%0.1f;%0.1f", mainPanelDevice:get_argument_value(251), mainPanelDevice:get_argument_value(252)))
	-- TACAN Channel
	SendDataESP8266(2263, string.format("%0.2f;%0.2f;%0.2f", mainPanelDevice:get_argument_value(263), mainPanelDevice:get_argument_value(264), mainPanelDevice:get_argument_value(265)))
end
--os.setlocale("ISO-8559-1", "numeric")

-- Simulation id
gESP8266SimID = string.format("%08x*",os.time())

-- State data for export
gESP8266PacketSize = 0
gESP8266SendStrings = {}
gESP8266LastData = {}

-- DCS Export Functions

do
--Hook
    LuaExportStart=function()
    -- Works once just before mission start.
    	
        -- 2) Setup udp sockets to talk to helios
        package.path  = package.path..";.\\LuaSocket\\?.lua"
        package.cpath = package.cpath..";.\\LuaSocket\\?.dll"
       
        socket = require("socket")
        
		--Set up connection
		udpESP8266 = socket.udp()
		udpESP8266:setpeername(gESP8266host, gESP8266Port)  -- this shound make it a connected session
        ip, port = udpESP8266:getsockname()
        print("NL Mods - ESP8266.lua sending to "..gESP8266host..":"..string.format("%i",gESP8266Port))
        print("NL Mods - ESP8266.lua receiving from "..ip..":"..string.format("%i",port))
		--udpListenerESP8266 = socket.udp()
		--udpListenerESP8266:setsockname("*", gESP8266Port+1) -- binds to a local address
		udpESP8266:settimeout(.001)
		--ip, port = udpListenerESP8266:getsockname()

		WriteToLogESP8266(string.format("Sockets created.\n"))

        if PrevExport.LuaExportStart then
            PrevExport.LuaExportStart()
        end
    end
end
do
	--Hook
	LuaExportBeforeNextFrame=function()
        ProcessInputESP8266()
        if PrevExport.LuaExportBeforeNextFrame then
            PrevExport.LuaExportBeforeNextFrame()
        end
    end
end

do
    --Hook
	LuaExportAfterNextFrame=function()
		if PrevExport.LuaExportAfterNextFrame  then
			PrevExport.LuaExportAfterNextFrame()
		end
	end
end

do
    --Hook
	LuaExportStop=function()
-- Works once just after mission stop.
        if udpESP8266 then
            udpESP8266:close()
        end
        --if udpListenerESP8266 then
        --    udpListenerESP8266:close()
        --end
        if logFileESP8266 then
    		logFileESP8266:flush()
    		logFileESP8266:close()
    		logFileESP8266 = nil
	    end
		if PrevExport.LuaExportStop  then
			PrevExport.LuaExportStop()
        end
    end
end

do
    --Hook
    LuaExportActivityNextEvent=function(t)
        -- print("NL Mods - Entered ESP8266 ExportActivityNextEvent.\n")

        -- WriteToLogESP8266("Entered LuaExportActivityNextEvent")
        local lt = t + gESP8266ExportInterval
        local olt   
        local lDevice = GetDevice(0)
        if type(lDevice) == "table" then
            lDevice:update_arguments()
    
            ProcessArgumentsESP8266(lDevice, gESP8266EveryFrameArguments)
            ProcessHighImportanceESP8266(lDevice)

            if gESP8266TickCount >= gESP8266ExportLowTickInterval then
                -- ProcessArgumentsESP8266(lDevice, gESP8266Arguments)
                ProcessLowImportanceESP8266(lDevice)
                gESP8266TickCount = 0
            end   
            FlushDataESP8266()
        end
        if PrevExport.LuaExportActivityNextEvent then
            olt = PrevExport.LuaExportActivityNextEvent(t)  -- if we were given a value then pass it on
        end

        if  lt > olt then 
            lt = olt -- take the lesser of the next event times
        end
        return lt
    end
end

function ProcessInputESP8266()
    -- local commandData, ip, port = udpListenerESP8266:receivefrom()
    local commandData = udpESP8266:receive()
    if commandData then
        WriteToLogESP8266(string.format("Received UDP data: %s", commandData))
        local commandTable = StrSplitESP8266(commandData, ",")

        for i, v in ipairs(commandTable) do 
            -- WriteToLogESP8266(string.format("Captured: %s, %s", i, v)) 
            end
        if #commandTable == 4 then
            -- WriteToLogESP8266(string.format("Split data into Device: %s Action: %s Value: %s", commandTable[2], commandTable[3], commandTable[4]))
            if commandTable[1] == "C" then
                --LoSetCommand
                if commandTable[2] == "-9999" then
                    -- WriteToLogESP8266(string.format("Sending LoSetCommand(%s, %s)", commandTable[3], commandTable[4]))
                    LoSetCommand(commandTable[3], commandTable[4])
                end
                local dev = GetDevice(commandTable[2])
                if dev then
                    dev:performClickableAction(commandTable[3], commandTable[4])
                end
                if commandTable[2] == 53 then --Send ILS twice because something weird is happening.
                    dev:performClickableAction(commandTable[3], commandTable[4])
                end
            else
                WriteToLogESP8266("UDP data was parsed into a four part command, but the first argument wasn't correct.")
            end
        else
            WriteToLogESP8266("Unable to parse UDP data into a four part command.");
        end
    end

end


-- Helper Functions
function StrSplitESP8266(str, delim, maxNb)
    -- Eliminate bad cases...
    if string.find(str, delim) == nil then
        return { str }
    end
    if maxNb == nil or maxNb < 1 then
        maxNb = 0    -- No limit
    end
    local result = {}
    local pat = "(.-)" .. delim .. "()"
    local nb = 0
    local lastPos
    for part, pos in string.gfind(str, pat) do
        nb = nb + 1
        result[nb] = part
        lastPos = pos
        if nb == maxNb then break end
    end
    -- Handle the last field
    if nb ~= maxNb then
        result[nb + 1] = string.sub(str, lastPos)
    end
    return result
end

function roundESP8266(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

-- Status Gathering Functions
function ProcessArgumentsESP8266(device, arguments)
	local lArgument , lFormat , lArgumentValue
		
	for lArgument, lFormat in pairs(arguments) do 
		lArgumentValue = string.format(lFormat,device:get_argument_value(lArgument))
		SendDataESP8266(lArgument, lArgumentValue)
	end
end

-- Network Functions
function SendDataESP8266(id, value)	
	if string.len(value) > 3 and value == string.sub("-0.00000000",1, string.len(value)) then
		value = value:sub(2)
	end
	
	if gESP8266LastData[id] == nil or gESP8266LastData[id] ~= value then
		local data =  id .. "=" .. value
		local dataLen = string.len(data)

		if dataLen + gESP8266PacketSize > 576 then
			FlushDataESP8266()
		end

		table.insert(gESP8266SendStrings, data)
		gESP8266LastData[id] = value	
		gESP8266PacketSize = gESP8266PacketSize + dataLen + 1
	end	
end

function FlushDataESP8266()

    if #gESP8266SendStrings > 0 then
		local packet = gESP8266SimID .. table.concat(gESP8266SendStrings, ":") .. "\n"
        socket.try(udpESP8266:send(packet))  -- this should be a connected session so no address is needed
		gESP8266SendStrings = {}
		gESP8266PacketSize = 0
	end
end

local gESP8266LastData = {}
	gESP8266TickCount = 10


print("NL Mods - ESP8266.lua script completed.\n")