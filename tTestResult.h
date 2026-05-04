#pragma once

class tTestResult {

public:
    
    // We have a limited number of values, so use the simple approach
    enum tValueType {
        None,
        Boolean,
        Float,
        Integer,
        String,
        InternalBoolean,
    };

private:
    // Values
    tValueType ValueType;
    bool ValueBool;
    double ValueFloat;
    int64_t ValueInteger;
    QString ValueString;
    bool InternalCheckValue = false;
    QString BoolPosValue = "true";
    QString BoolNegValue = "false";
    int FloatPrecision = 3;
protected:
    bool ValueSet = false;
    //QString ResultComment = "";

public:
    void Clear() { 
        ValueSet = false; 
        //ResultComment = "";
        //ValueType = tValueType::None; ?
    }
    bool IsValueSet() const { return ValueSet; }
    void SetValueType(tValueType type) { 
        qDebug() << "SetValueType to" << type;
        ValueType = type; 
    }
    tValueType GetValueType() const { 
        return ValueType; 
    }
    void SetPrecision(int prec) { FloatPrecision = prec; }
    void SetBoolValues(const QString& posVal, const QString& negVal) { BoolPosValue = posVal; BoolNegValue = negVal; }
    void SetBoolValues(const QString& posVal) { // Automatically detects pair of bool values
        // TODO
    };
    bool SetValue(bool value/*, const QString &comment = ""*/) {
        if (ValueSet) return false; // Value can be set only once
        if (ValueType == tValueType::Boolean) {
            ValueBool = value;
            ValueSet = true;
            //ResultComment = comment;
            return true;
        } else {
            return false; // Can set value only once
        }
    }

    bool SetValue(int64_t value/*, const QString& comment = ""*/) {
        if (ValueSet) return false; // Value can be set only once
        if (ValueType == tValueType::Integer) {
            ValueInteger = value;
            ValueSet = true;
            //ResultComment = comment;
            return true;
        } else {
            return false; // Can set value only once
        }
    }

    bool SetValue(double value /*const QString& comment = ""*/) {
        if (ValueSet) return false; // Value can be set only once
        if (ValueType == tValueType::Float) {
            ValueFloat = value;
            ValueSet = true;
            //ResultComment = comment;
            return true;
        } else {
            return false; // Can set value only once
        }
    }

    bool SetValue(QString value/*, const QString& comment = ""*/) {
        if (ValueSet) return false; // Value can be set only once
        if (ValueType == tValueType::String) {
            ValueString = value;
            ValueSet = true;
            //ResultComment = comment;
            return true;
        } else {
            return false; // Can set value only once
        }
    }

    bool GetInternalCheckValue() const {
        return InternalCheckValue;
    }

    void SetInternalCheckValue(bool value) {
        InternalCheckValue = value;
    }

    bool GetValue(bool& res) const { // TODO: when types do not match, set TEST ERROR status
        if (!ValueSet) return false;
        if (ValueType != tValueType::Boolean) return false;
        res = ValueBool;
        return true;
    }

    bool GetValue(double& res) const { // TODO: when types do not match, set TEST ERROR status
        if (!ValueSet) return false;
        if (ValueType != tValueType::Float) return false;
        res = ValueFloat;
        return true;
    }

    bool GetValue(int64_t& res) const { // TODO: when types do not match, set TEST ERROR status
        if (!ValueSet) return false;
        if (ValueType != tValueType::Integer) return false;
        res = ValueInteger;
        return true;
    }

    bool GetValue(QString& res) const { // TODO: when types do not match, set TEST ERROR status
        if (!ValueSet) return false;
        if (ValueType != tValueType::String) return false;
        res = ValueString;
        return true;
    }

    QString ToString() const {
        QString res;
        switch (ValueType) {
        case tValueType::Boolean: 
            if (ValueBool) res = BoolPosValue; else res = BoolNegValue;
            break;
        case tValueType::Float: 
            res = QString("%1").arg(ValueFloat, 0, 'f', FloatPrecision);
            break;
        case tValueType::Integer: 
            res = QString("%1").arg(ValueInteger);
            break;
        case tValueType::None: 
            res = "";
            break;
        default:;
        }
        return res;
    }

    //QString GetResultComment() const {
    //    return "";// ResultComment;
    //}

    //void SetResultComment(QString comments) {
        //ResultComment = comments;
    //}

};
