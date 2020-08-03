
#include "SecuredMenuPopup.h"
#include "tcMenu.h"
#include "BaseDialog.h"

const char pgmProceedText[] PROGMEM = "Proceed";
const char pgmCancelText[] PROGMEM = "Cancel";
const char pgmHeaderNotAuth[] PROGMEM = "Pin incorrect";

RENDERING_CALLBACK_NAME_INVOKE(fnpopupPasswordRtCall, textItemRenderFn, "Password", -1, NULL)

SecuredMenuPopup::SecuredMenuPopup(AuthenticationManager * authentication) 
	: pinEntryItem(fnpopupPasswordRtCall, nextRandomId(), MAX_PIN_LENGTH, &actionProceedItem),
		actionProceedItem(secPopupActionRenderFn, 1, &actionCancelItem),
		actionCancelItem(secPopupActionRenderFn, 0, NULL) {

	this->authentication = authentication;
}

MenuItem* SecuredMenuPopup::start(SubMenuItem* securedMenu) {
	actionProceedItem.setSecuredItem(securedMenu);
	actionProceedItem.setActive(false);
	actionCancelItem.setSecuredItem(NULL);
	actionCancelItem.setActive(false);

	pinEntryItem.setTextValue("", true);
	pinEntryItem.setPasswordField(true);
	pinEntryItem.setActive(true);
	pinEntryItem.setEditing(false);
	return &pinEntryItem;
}

int secPopupActionRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_ACTIVATE_SUBMENU) return false;
	ActivateSubMenuItem* act = reinterpret_cast<ActivateSubMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: 
		buffer[0] = 0;
		return true;
	case RENDERFN_NAME:
		strncpy_P(buffer, (row != 0) ? pgmProceedText : pgmCancelText, bufferSize);
		return true;
	case RENDERFN_EEPROM_POS:
		return -1;
	case RENDERFN_INVOKE:
		if(act->getSecuredItem() != NULL) {
			if (menuMgr.secureMenuInstance()->doesPinMatch()) {
				menuMgr.setCurrentMenu(act->getSecuredItem());
			}
			else {
				BaseDialog* dlg = menuMgr.getRenderer()->getDialog();
				dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
				char sz[15];
				act->getSecuredItem()->copyNameToBuffer(sz, sizeof(sz));
				dlg->show(pgmHeaderNotAuth, false);
				dlg->copyIntoBuffer(sz);
				menuMgr.setCurrentMenu(menuMgr.getRoot());
			}
		}
		else {
			menuMgr.setCurrentMenu(menuMgr.getRoot());
		}
	default:
		return false;
	}

}
