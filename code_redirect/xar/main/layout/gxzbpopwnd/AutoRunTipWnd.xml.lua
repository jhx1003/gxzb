local tFunctionHelper = XLGetGlobal("Global.FunctionHelper")
local tUserConfig = tFunctionHelper.ReadConfigFromMemByKey("tUserConfig") or {}
local g_CheckBoxState = true
local strAutoRunRegPath = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\Share4Money"

function CheckIsAutoRun()
	local strValue = Helper:QueryRegValue(strAutoRunRegPath)
	if Helper:IsRealString(strValue) then
		return true
	end
	return false
end

function SetAutoRun()
	local strExePath = tFunctionHelper.GetExePath()
	local strValue = "\""..strExePath.."\" /sstartfrom sysboot /embedding /mining"
	Helper:SetRegValue(strAutoRunRegPath, strValue)
end

function DoWorkOnHideWnd()
	if g_CheckBoxState and not CheckIsAutoRun() then
		
		SetAutoRun()
	end	
end

function OnClickClose(self)
	local objTree = self:GetOwner()
	local objHostWnd = objTree:GetBindHostWnd()
	objHostWnd:Show(0)
	DoWorkOnHideWnd()
end

function PopupInDeskRight(self)
	local objtree = self:GetBindUIObjectTree()
	local objRootLayout = objtree:GetUIObject("root")
    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
	
	local nLayoutL, nLayoutT, nLayoutR, nLayoutB = objRootLayout:GetObjPos()
	local nLayoutWidth = nLayoutR - nLayoutL
	local nLayoutHeight = nLayoutB - nLayoutT
	
	local workleft, worktop, workright, workbottom = Helper.tipUtil:GetWorkArea()
	self:Move( workright - nLayoutWidth, workbottom - nLayoutHeight, nLayoutWidth, nLayoutHeight)
	return true
end

function OnClickBind(self)
	local objTree = self:GetOwner()
	local objHostWnd = objTree:GetBindHostWnd()
	objHostWnd:Show(0)
	local mainwnd = tFunctionHelper.GetMainHostWnd()
	if mainwnd then
		mainwnd:BringWindowToTop(true)
	end
	tFunctionHelper.ChangeMainBodyPanel("QRCodePanel")
	DoWorkOnHideWnd()
end

function OnSelectAutoRun(self, event, bSelect)
	if bSelect then
		g_CheckBoxState = true
	else
		g_CheckBoxState = false
	end	
end

function OnShowWindow(self, bShow)
	if bShow then
		if not tFunctionHelper.CheckIsInitPopupTipWnd() then
			return
		end
		local objtree = self:GetBindUIObjectTree()
		PopupInDeskRight(self)
		
		local objIcon = objtree:GetUIObject("AutoRunTipWnd.Icon")
		local objTextContentBegain = objtree:GetUIObject("AutoRunTipWnd.Content.Begain")
		local objTextContentDesc = objtree:GetUIObject("AutoRunTipWnd.Content.Desc")
		local objTextBind = objtree:GetUIObject("AutoRunTipWnd.Bind")
		local objCheckAutoRun = objtree:GetUIObject("AutoRunTipWnd.CheckAutoRun")
		if tFunctionHelper.CheckIsBinded() then
			objTextContentBegain:SetObjPos(160, 92+20, 160+310, 92+20+20)
			objTextContentDesc:SetObjPos(160, 92+20+20, 160+310, 92+20+20+20)
			objTextBind:SetChildrenVisible(false)
			objTextBind:SetVisible(false)
		else
			objTextContentBegain:SetObjPos(160, 92+10, 160+310, 92+10+20)
			objTextContentDesc:SetObjPos(160, 92+10+20, 160+310, 92+10+20+20)
			objTextBind:SetObjPos(160, 92+10+20+20, 160+310, 92+10+20+20+20)
			objTextBind:SetChildrenVisible(true)
			objTextBind:SetVisible(true)
		end
		if CheckIsAutoRun() then
			g_CheckBoxState = false
			objCheckAutoRun:SetCheck(false)
			objCheckAutoRun:SetChildrenVisible(false)
			objCheckAutoRun:SetVisible(false)
		else
			g_CheckBoxState = true
			objCheckAutoRun:SetCheck(true)
			objCheckAutoRun:SetChildrenVisible(true)
			objCheckAutoRun:SetVisible(true)
		end
		local nTipHolds = 10
		if type(g_ServerConfig) == "table" and type(g_ServerConfig["tRemindCfg"]) == "table" and type(g_ServerConfig["tRemindCfg"]["nHolds"]) == "number" then
			nTipHolds = g_ServerConfig["tRemindCfg"]["nHolds"]
		end
		SetOnceTimer(function(item, id)
			self:Show(0)
			DoWorkOnHideWnd()
		end, nTipHolds*1000)
	end
end