
local lfs = require('lfs')
LUA_PATH = "?;?.lua;"..lfs.currentdir().."/Scripts/?.lua"
-- require 'Vector'
-- See the Scripts\Vector.lua file for Vector class details, please.

local default_output_file = nil

function LuaExportStart()
-- Works once just before mission start.
-- Make initializations of your files or connections here.
-- For example:
-- 1) File
  default_output_file = io.open("C:/Users/kerrs_000/Saved Games/DCS/Logs/Export.log", "w")
  default_output_file.write("Hello world!\n");
-- 2) Socket
	package.path  = package.path..";"..lfs.currentdir().."/LuaSocket/?.lua"
	package.cpath = package.cpath..";"..lfs.currentdir().."/LuaSocket/?.dll"
	socket = require("socket")
	host = host or "localhost"
	port = port or 1337
	c = socket.try(socket.connect(host, port)) -- connect to the listener socket
	c:setoption("tcp-nodelay",true) -- set immediate transmission mode
	
	local version = LoGetVersionInfo() --request current version info (as it showed by Windows Explorer fo DCS.exe properties)
	  if version and default_output_file then
	    
		default_output_file:write("ProductName: "..version.ProductName..'\n')
		default_output_file:write(string.format("FileVersion: %d.%d.%d.%d\n",
												version.FileVersion[1],
												version.FileVersion[2],
												version.FileVersion[3],
												version.FileVersion[4]))
		default_output_file:write(string.format("ProductVersion: %d.%d.%d.%d\n",
												version.ProductVersion[1],
												version.ProductVersion[2],
												version.ProductVersion[3],  -- head  revision (Continuously growth)
												version.ProductVersion[4])) -- build number   (Continuously growth)	
	  end

end

function LuaExportBeforeNextFrame()
-- Works just before every simulation frame.

-- Call Lo*() functions to set data to Lock On here
-- For example:
--	LoSetCommand(3, 0.25) -- rudder 0.25 right 
--	LoSetCommand(64) -- increase thrust

end

function GetType(id)

	local emitter		= LoGetObjectById(id)

    local unknown, airborne, long_range, medium_range, short_range, early_wrning, awacs = -1, 0, 1, 2, 3, 4, 5;

	local lNameByType = LoGetNameByType(emitter.Type.level1, emitter.Type.level2, emitter.Type.level3, emitter.Type.level4)

    -- threat type
	if (emitter.Type.level1 == 1 or emitter.Type.level1 == 4) and 
	not(emitter.Type.level4 == 26 or --.Type.level4 26: A-50
	emitter.Type.level4 == 27 or --.Type.level4 27: E-3
	emitter.Type.level4 == 41) then --.Type.level4 41: E-2C
		return airborne;
	end

	if emitter.Type.level1 == 2 or emitter.Type.level1 == 3 then
		-- ground or navy
		if emitter.Type.level2 == 16 then
			local lAn, lEn
			-- Ground SAM
			lAn, lEn = string.find("s300 sr,s300 tr,an/mpq-53", lNameByType, 0, true)
			-- long range radar
			if lAn ~= nil then
				return long_range;
			end

			lAn, lEn = string.find("p19,snr-125,kub,hawk sr,hawk tr,buk sr,buk tr,5n66m sr", lNameByType, 0, true)
			-- mid range radar
			if lAn ~= nil then
				return medium_range
			end

			lAn, lEn = string.find("M163 Vulcan,shilka zsu-23-4,gepard,roland ads,roland radar,osa 9a33 ln,2c6m,9a331,Dog Ear Radar", lNameByType, 0, true)
			-- short range radar
			if lAn ~= nil then
				return short_range
			end
--[[			if lType.Type.level4 == 27 or -- Dog Ear Radar
			   lType.Type.level4 == 31 or -- roland ads
			   lType.Type.level4 == 32 or -- roland radar
			   lType.Type.level4 == 38 then -- gepard
				lReturn.SRR = 1
			end
]]
			lAn, lEn = string.find("1l13 ewr station,55G6", lNameByType, 0, true)
			-- EWR
			if lAn ~= nil then
				return early_wrning
			end

		elseif emitter.Type.level2 == 12 then
			local lAn, lEn
			-- Ship
			lAn, lEn = string.find("FFG-7 Oliver H. Perry class,SG-47 Ticonderoga class", lNameByType, 0, true)
			-- long range radar
			if lAn ~= nil then
				return long_range
			end

			lAn, lEn = string.find("CVN-70 Vinson", lNameByType, 0, true)
			-- short range radar
			if lAn ~= nil then
				return short_range
			end
		end
	elseif emitter.Type.level1 == 1 and emitter.Type.level2 == 1 and emitter.Type.level3 == 5 then 
	     if emitter.Type.level4 == 26 or emitter.Type.level4 == 27 or emitter.Type.level4 == 41 then
		    -- AWACS
		    --.Type.level4 26: A-50
		    --.Type.level4 27: E-3
		    --.Type.level4 41: E-2C
		    return awacs
		end
	end

	if default_output_file then
		default_output_file:write("\nUnknown type:")

		if emitter.Type.level1 then
			default_output_file:write(string.format("%d", emitter.Type.level1))
		else
			default_output_file:write("nil ")
		end

		if emitter.Type.level2 then
			default_output_file:write(string.format("%d", emitter.Type.level2))
		else
			default_output_file:write("nil ")
		end

		if emitter.Type.level3 then
			default_output_file:write(string.format("%d", emitter.Type.level3))
		else
			default_output_file:write("nil ")
		end

		if emitter.Type.level4 then
			default_output_file:write(string.format("%d", emitter.Type.level4))
		else
			default_output_file:write("nil ")
		end

		default_output_file:write("\n")

	end
   	return unknown
end

function GetAbove(id)

	local emitterObject		= LoGetObjectById(id)
	local emitterObjectAlt	= 0

	if emitterObject then
		emitterObjectAlt = emitterObject.LatLongAlt.Alt
	end

	local lSelfData		= LoGetSelfData()
	local lSelfDataAlt	= lSelfData.LatLongAlt.Alt

	local above = 0
	if emitterObjectAlt and lSelfDataAlt then
		if emitterObjectAlt >= (lSelfDataAlt - 200) then
			above = 1
		end
	end

	return above
end

function GetBelow(id)

	local emitterObject		= LoGetObjectById(id)
	local emitterObjectAlt	= 0

	if emitterObject then
		emitterObjectAlt = emitterObject.LatLongAlt.Alt
	end

	local lSelfData		= LoGetSelfData()
	local lSelfDataAlt	= lSelfData.LatLongAlt.Alt

	local below = 0
	if emitterObjectAlt and lSelfDataAlt then
		if emitterObjectAlt <= (lSelfDataAlt + 200) then
			below = 1
		end
	end

	return below

end

function LuaExportAfterNextFrame()
-- Works just after every simulation frame.

	radarActive = LoGetSelfData().Flags.RadarActive

	local threats = LoGetTWSInfo()
	-- Call Lo*() functions to get data from Lock On here.
	-- For example:
	--	local t = LoGetModelTime()
	--	local name = LoGetPilotName()
	--	local altBar = LoGetAltitudeAboveSeaLevel()
	--	local altRad = LoGetAltitudeAboveGroundLevel()
	--	local pitch, bank, yaw = LoGetADIPitchBankYaw()
	--	local engine = LoGetEngineInfo()
	--	local HSI    = LoGetControlPanel_HSI()
	-- Then send data to your file or to your receiving program:
	-- 1) File
	if threats then		
		-- add emiters to json
		local jsonEmitters = "[ "
		for mode,emit in pairs (threats.Emitters) do
			local jsonEmit = ""
			jsonEmit = string.format('{ "ID":"%s", "Power":%f, "Azimuth":%f, "Priority":%f, "SignalType":"%s", "Type":%d, "Above":%d, "Below":%d}', emit.ID, emit.Power, emit.Azimuth, emit.Priority, emit.SignalType, GetType(emit.ID), GetAbove(emit.ID), GetBelow(emit.ID))
			if jsonEmitters ~= "[ " then
				jsonEmitters = jsonEmitters .. ","
			end
			jsonEmitters = jsonEmitters .. jsonEmit
			if default_output_file then
				default_output_file:write(jsonEmit)
			end
		end
		jsonEmitters = jsonEmitters .. "]"
		
		local radarActiveValue = 0
		if radarActive then
			radarActiveValue = 1
		end

		jsonThreats = string.format('{ "Mode":%f, "Emitters":%s, "RadarOn":%d }', threats.Mode, jsonEmitters, radarActiveValue)
		if default_output_file then
			default_output_file:write(jsonThreats)
			default_output_file:write('\n')
		end

		if c then
			socket.try(c:send(jsonThreats))
		end
	end

	if default_output_file then
		--default_output_file:write(string.format("t = %.2f, name = %s, altBar = %.2f, altRad = %.2f, pitch = %.2f, bank = %.2f, yaw = %.2f\n", t, name, altBar, altRad, 57.3*pitch, 57.3*bank, 57.3*yaw))
		--default_output_file:write(string.format("t = %.2f ,RPM left = %f  fuel_internal = %f \n",t,engine.RPM.left,engine.fuel_internal))
	--	default_output_file:write(string.format("ADF = %f  RMI = %f\n ",57.3*HSI.ADF,57.3*HSI.RMI))
	--	 default_output_file:write(string.format("", ))
	end
	-- 2) Socket
		--socket.try(c:send(string.format("t = %.2f, name = %s, altBar = %.2f, alrRad = %.2f, pitch = %.2f, bank = %.2f, yaw = %.2f\n", t, name, altRad, altBar, pitch, bank, yaw)))

end

function LuaExportStop()
-- Works once just after mission stop.
-- Close files and/or connections here.
-- 1) File
   if default_output_file then
      default_output_file:write("Done!\n")
	  default_output_file:close()
	  default_output_file = nil
   end
-- 2) Socket
	--socket.try(c:send("quit")) -- to close the listener socket
	c:close()
end

function LuaExportActivityNextEvent(t)
	local tNext = t

	return tNext
end

