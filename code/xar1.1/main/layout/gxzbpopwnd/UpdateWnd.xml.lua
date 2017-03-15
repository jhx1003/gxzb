local tFunctionHelper = XLGetGlobal("Global.FunctionHelper")
local tipAsynUtil = XLGetObject("API.AsynUtil")
local g_tNewVersionInfo = nil
--0未更新， 1正在更新
local g_UpdateState = 0
--是否被取消
local g_UpdateCancel = false

function OnClickClose(self)
	local objTree = self:GetOwner()
	local objHostWnd = objTree:GetBindHostWnd()
	if g_UpdateState == 1 then
		local nRet = Helper:CreateModalWnd("MessageBoxWnd", "MessageBoxWndTree", nil, 
			{
				["parentWnd"] = objHostWnd, 
				["Text"] = "您正在升级，是否退出？",
				["ChangeUI"] = function(objWnd)
					local objtree = objWnd:GetBindUIObjectTree()
					local btnyes = objtree:GetUIObject("yes")
					local btnno = objtree:GetUIObject("no")
					btnyes:SetText("确定")
					btnno:SetText("取消")
				end,
			}
		)
		--点了取消或关闭
		if nRet == 1 then
			return
		--点了退出
		else
			g_UpdateState = 0
			g_UpdateCancel = true
		end
	end
	objHostWnd:EndDialog(0)
end

function CheckPacketMD5(strPacketPath)
	local strServerMD5 = g_tNewVersionInfo["strMD5"]
	return FunctionObj.CheckMD5(strPacketPath, strServerMD5)
end


local progress, progrBar, progrText
function OnClickUpdateBtn(self)
	if not g_tNewVersionInfo then 
		return 
	end
	--local TextBig = self:GetObject("tree:UpdateWnd.Content.TextBig")
	--local TextVersion = objtree:GetUIObject("UpdateWnd.Content.TextVersion")
	local TextMain = self:GetObject("tree:UpdateWnd.Content.TextMain")
	local BtnUpdate = self:GetObject("tree:UpdateWnd.OneKeyUpdate.Btn")
	progress = self:GetObject("tree:UpdateWnd.Progress.bkg")
	progrBar = self:GetObject("tree:UpdateWnd.Progress.bar")
	progrText = self:GetObject("tree:UpdateWnd.Progress.text")
	TextMain:SetVisible(false)
	--TextVersion:SetVisible(false)
	BtnUpdate:Show(false)
	progress:SetVisible(true)
	progress:SetChildrenVisible(true)
	progrBar:SetVisible(true)
	progrText:SetVisible(true)
	--TextBig:SetObjPos(158, 100, "father.width", 136)
	--正在更新
	g_UpdateState = 1
	if g_UpdateCancel then
		g_UpdateCancel = false
		return
	end
	local strCurVersion = tFunctionHelper.GetGXZBVersion()
	local strNewVersion = g_tNewVersionInfo.strVersion		
	if not Helper:IsRealString(strCurVersion) or not Helper:IsRealString(strNewVersion)
		or not tFunctionHelper.CheckIsNewVersion(strNewVersion, strCurVersion) then
		tFunctionHelper.TipLog("[OnClickUpdateBtn] strCurVersion is nil or is not New Version")
		return
	end
	local strUrl = g_tNewVersionInfo["strPacketURL"]
	local strFileName = tFunctionHelper.GetFileSaveNameFromUrl(strUrl)
	if not string.find(strFileName, "%.exe$") then
		strFileName = strFileName..".exe"
	end
	local strSaveDir = Helper.tipUtil:GetSystemTempPath()
	local strSavePath = Helper.tipUtil:PathCombine(strSaveDir, strFileName)
	
	tFunctionHelper.TipLog("[OnClickUpdateBtn] strUrl = "..tostring(strUrl)..", strSavePath = "..tostring(strSavePath))
	tipAsynUtil:AsynGetHttpFileWithProgress(strUrl, strSavePath, false, function(nRet, savepath, ulProgress, ulProgressMax)
		if g_UpdateCancel then
			if nRet == 0 then
				g_UpdateCancel = false
			end
			tFunctionHelper.TipLog("[OnClickUpdateBtn] AsynGetHttpFileWithProgress g_UpdateCancel = "..tostring(g_UpdateCancel))
			return
		end
		tFunctionHelper.TipLog("[OnClickUpdateBtn] AsynGetHttpFileWithProgress nRet = "..tostring(nRet)..", ulProgress = "..tostring(ulProgress)..", ulProgressMax = "..tostring(ulProgressMax))
		if nRet == -2 and type(ulProgress) == "number" and type(ulProgressMax) == "number" and ulProgress < ulProgressMax and ulProgress > 0 then
			local rate = ulProgress/ulProgressMax
			local rateText = tostring(math.floor(rate*100)).."%"
			local l, t, r, b = progrBar:GetObjPos()
			local fl, ft, fr, fb = progress:GetObjPos()
			local w = fr - fl - 2
			progrBar:SetObjPos(l, t, l + w*rate, b)
			progrText:SetText("正在下载"..rateText)
		elseif nRet == 0 and  Helper.tipUtil:QueryFileExists(savepath) and CheckPacketMD5(savepath) then
			tipUtil:ShellExecute(0, "open", savepath, 0, 0, "SW_SHOWNORMAL")
		end
	end)
end

function GetPacketSavePath(strURL)
	if not Helper:IsRealString(strURL) then
		return 
	end
	local strFileName = tFunctionHelper.GetFileSaveNameFromUrl(strURL)
	if not string.find(strFileName, "%.exe$") then
		strFileName = strFileName..".exe"
	end
	local strSaveDir = Helper.tipUtil:GetSystemTempPath()
	local strSavePath = Helper.tipUtil:PathCombine(strSaveDir, strFileName)

	return strSavePath
end

function OnCreate(self)
	local userData = self:GetUserData()
	if userData and userData.parentWnd then
		local objtree = self:GetBindUIObjectTree()
		local objRootLayout = objtree:GetUIObject("root")
		local nLayoutL, nLayoutT, nLayoutR, nLayoutB = objRootLayout:GetObjPos()
		local nLayoutWidth  = nLayoutR - nLayoutL
		local nLayoutHeight = nLayoutB - nLayoutT
	
		local parentLeft, parentTop, parentRight, parentBottom = userData.parentWnd:GetWindowRect()
		local parentWidth  = parentRight - parentLeft
		local parentHeight = parentBottom - parentTop
		self:Move( parentLeft + (parentWidth - nLayoutWidth)/2, parentTop + (parentHeight - nLayoutHeight)/2, nLayoutWidth, nLayoutHeight)
		
		local TextBig = objtree:GetUIObject("UpdateWnd.Content.TextBig")
		local TextMain = objtree:GetUIObject("UpdateWnd.Content.TextMain")
		local TextVersion = objtree:GetUIObject("UpdateWnd.Content.TextVersion")
		local BtnUpdate = objtree:GetUIObject("UpdateWnd.OneKeyUpdate.Btn")
		TextMain:SetMultilineTextLimitWidth(260)
		--查询更新
		TextBig:SetVisible(false)
		BtnUpdate:Show(false)
		--TextVersion:SetVisible(false)
		TextMain:SetText("正在为您检查更新，请稍后...")
		
		local function ShowNoUpdate()
			--已经是最新
			TextMain:SetText("您的共享赚宝已经是最新版本。")
		end
		
		local function ShowReadyUpdate(strVersion, strContent)
			--发现新版本
			TextBig:SetVisible(true)
			TextBig:SetText("发现共享赚宝新版本"..tostring(strVersion))
			TextMain:SetVisible(false)
			TextVersion:SetVisible(true)
			BtnUpdate:Show(true)
			TextVersion:SetText(strContent)
			local w, h = TextVersion:GetTextExtent()
			local Hoffset =  h - 130
			if Hoffset > 0 then
				TextVersion:SetObjPos(158, 95, 158+260, 95 + h + 10)
				local wndL, wndT, wndR, wndB = self:GetWindowRect()
				self:Move(wndL, wndT, wndR - wndL, wndB - wndT + Hoffset)
				objRootLayout:SetObjPos(0, 0, wndR - wndL, wndB - wndT + Hoffset)
				self:SetMaxTrackSize(wndR - wndL, wndB - wndT + Hoffset)
			end
		end
		
		local function InitMainWnd(nRet, strCfgPath)			
			--[[tNewVersionInfo = {
				strVersion = "v1.0.0.2",
				strContent = "1、修改了XX\n2、优化了xx\n3、see more",
				strPacketURL = "http://down.sandai.net/thunder9/Thunder9.1.26.614.exe",
				strMD5 = "1221212",
			}
			ShowReadyUpdate(tNewVersionInfo["strVersion"] or "V3.4.56.1", tNewVersionInfo["strContent"] or "1、修改了XX\n2、优化了xx\n3、see more")
			g_tNewVersionInfo = tNewVersionInfo
			if true then return end]]
			
			if 0 ~= nRet then
				ShowNoUpdate()
				return
			end	

			local tServerConfig = tFunctionHelper.LoadTableFromFile(strCfgPath) or {}
			local tNewVersionInfo = tServerConfig["tNewVersionInfo"] or {}
			local strPacketURL = tNewVersionInfo["strPacketURL"]
			if not Helper:IsRealString(strPacketURL) then
				ShowNoUpdate()
				return 
			end
			
			local strCurVersion = tFunctionHelper.GetGXZBVersion()
			local strNewVersion = tNewVersionInfo.strVersion
			if not Helper:IsRealString(strCurVersion) or not Helper:IsRealString(strNewVersion)
				or not tFunctionHelper.CheckIsNewVersion(strNewVersion, strCurVersion) then
				ShowNoUpdate(objRootCtrl)
				return
			end
			local strSavePath = GetPacketSavePath(strPacketURL)
			if Helper:IsRealString(tNewVersionInfo.strMD5) 
				and tFunctionHelper.CheckMD5(strSavePath, tNewVersionInfo.strMD5) then
				--ShowInstallPanel(objRootCtrl, strSavePath, tNewVersionInfo)
				tipUtil:ShellExecute(0, "open", strSavePath, 0, 0, "SW_SHOWNORMAL")
				return
			end
			ShowReadyUpdate(tNewVersionInfo["strVersion"] or "V3.4.56.1", tNewVersionInfo["strContent"] or "1、修改了XX\n2、优化了xx\n3、see more")
			g_tNewVersionInfo = tNewVersionInfo
		end
		tFunctionHelper.DownLoadServerConfig(InitMainWnd)
	end
end