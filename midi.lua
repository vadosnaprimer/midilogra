--
function logs(s)
  local logfile = io.open("midilogra.txt", "a+")
  logfile:write(s)
  logfile:close()
end

function dooutput (msg)
  local timestr = msg[1]
  
  local i = 3
  
  while (i <= #msg) do
    local evtype = msg[i]:match ("^.")
	--io.write ("##", evtype, "\n")
    if evtype == "c" or evtype == "d" then
	  logs (timestr..",".."0000"..msg[i + 1]..msg[i].."\n")
	  i = i + 2
	else
	  logs (timestr..",".."00"..msg[i + 2]..msg[i + 1]..msg[i].."\n")
	  i = i + 3
	end
  end
end


for line in io.lines ("midilog.txt") do
  -- for now, try ignoring special messages
  if line:find ("sysex") then
    local msg = {}
	for token in line:gmatch ("([^,]+)") do
	  table.insert (msg, token)
	end
	dooutput (msg)
  elseif line:find ("finalizing...") then
    local timestr = line:match ("(%d+)")
	local channums = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"}
	for i, v in ipairs (channums) do
	  logs (timestr..",00007bb"..v.."\n") -- all notes off
	end	
  end
end

  