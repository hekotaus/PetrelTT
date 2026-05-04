#include "PetrelTools.h"

QString IsValid(bool condition, bool& res) {
    res = res && condition;
    if (condition) return " -- Valid"; else return " -- Invalid";
}

QString XmlAttribute(tinyxml2::XMLElement* el, QString name, QString defValue) {
    const char* pAtt = el->Attribute(name.toStdString().c_str());
    QString res;
    if (pAtt == nullptr) res = defValue;
    else res = QString("%1").arg(pAtt);
    return res;
}
