#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"
#include "audio.h"

#include <QSize>
#include <QFile>

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male, bool hidden, bool never_shown)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), gender(male ? Male : Female), hidden(hidden), never_shown(never_shown)
{
    static QChar lord_symbol('$');
    if(name.contains(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    }else{
        lord = false;
        setObjectName(name);
    }
}

int General::getMaxHp() const{
    return max_hp;
}

QString General::getKingdom() const{
    return kingdom;
}

bool General::isMale() const{
    return gender == Male;
}

bool General::isFemale() const{
    return gender == Female;
}

bool General::isNeuter() const{
    return gender == Neuter;
}

void General::setGender(Gender gender){
    this->gender = gender;
}

General::Gender General::getGender() const{
    return gender;
}

QString General::getGenderString() const{
    switch(gender){
    case Male: return "male";
    case Female: return "female";
    default:
        return "neuter";
    }
}

bool General::isLord() const{
    return lord;
}

bool General::isHidden() const{
    return hidden;
}

bool General::isTotallyHidden() const{
    return never_shown;
}

QString General::getPixmapPath(const QString &category) const{
    QString suffix = "png";
    QString categOry = category;
    if(categOry.startsWith("card"))
        suffix = "jpg";

    QString path = QString("image/generals/%1/%2.%3").arg(categOry).arg(objectName()).arg(suffix);
    if(categOry == "card3" && !QFile::exists(path)){
        categOry = "card2";
        path = QString("image/generals/%1/%2.%3").arg(categOry).arg(objectName()).arg(suffix);
    }
    if(categOry == "card2" && !QFile::exists(path))
        path = QString("image/generals/card/%1.%2").arg(objectName()).arg(suffix);
    return path;
}

void General::addSkill(Skill *skill){
    skill->setParent(this);
    skill_set << skill->objectName();
}

void General::addSkill(const QString &skill_name){
    extra_set << skill_name;
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_set.contains(skill_name) || extra_set.contains(skill_name);
}

QList<const Skill *> General::getVisibleSkillList() const{
    QList<const Skill *> skills;
    foreach(const Skill *skill, findChildren<const Skill *>()){
        if(skill->isVisible())
            skills << skill;
    }

    foreach(QString skill_name, extra_set){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(!skill)
            skill = new Skill(skill_name);
        if(skill && skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const Skill *> General::getVisibleSkills() const{
    QSet<const Skill *> skills;
    foreach(const Skill *skill, findChildren<const Skill *>()){
        if(skill->isVisible())
            skills << skill;
    }

    foreach(QString skill_name, extra_set){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const TriggerSkill *> General::getTriggerSkills() const{
    QSet<const TriggerSkill *> skills = findChildren<const TriggerSkill *>().toSet();

    foreach(QString skill_name, extra_set){
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if(skill)
            skills << skill;
    }

    return skills;
}

void General::addRelateSkill(const QString &skill_name){
    related_skills << skill_name;
}

QStringList General::getRelatedSkillNames() const{
    return related_skills;
}

QString General::getPackage() const{
    QObject *p = parent();
    if(p)
        return p->objectName();
    else
        return QString(); // avoid null pointer exception;
}

QString General::getSkillDescription() const{
    QString description;

    foreach(const Skill *skill, getVisibleSkillList()){
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    return description;
}

static QString GetAudioPath(const QString &dirname, QString name){
    static QList<QRegExp> rxs;
    if(rxs.isEmpty()){
        rxs << QRegExp("(.*)") << QRegExp(".*_(.*)") << QRegExp("(.*)f");
    }

    foreach(QRegExp rx, rxs){
        QString path = Audio::audioPath(dirname, name.replace(rx, "\\1"));
        if(!path.isNull())
            return path;
    }

    return QString();
}

QString General::getWinEffectPath() const{
    // special case
    if(isCaoCao())
        return "audio/win/caocao.ogg";
    else
        return GetAudioPath("audio/win", objectName());
}

QString General::getLastEffectPath() const{
    return GetAudioPath("audio/death", objectName());
}

void General::playLastWord() const{
    QString filename = getLastEffectPath();
    Sanguosha->playEffect(filename);
}

void General::playWinWord() const{
    Sanguosha->playEffect(getWinEffectPath());
}

QString GetWord(QString name, const QString &prefix){
    static QList<QRegExp> rxs;
    if(rxs.isEmpty()){
        rxs << QRegExp("(.*)") << QRegExp(".*_(.*)") << QRegExp("(.*)f");
    }

    foreach(QRegExp rx, rxs){
        QString key = prefix + name.replace(rx, "\\1");
        QString value = Sanguosha->translate(key, QString());
        if(!value.isNull())
            return value;
    }

    return QString();
}

QString General::getLastWord() const{
    return GetWord(objectName(), "~");
}

QString General::getWinWord() const{
    return GetWord(objectName(), "`");
}

bool General::isCaoCao() const{
    return objectName().contains("caocao") || objectName() == "weiwudi";
}

bool General::isZhugeliang() const
{
    return objectName().contains("zhugeliang") || objectName() == "wolong";
}

bool General::nameContains(const QString &name) const
{
    return objectName().contains(name);
}

QString General::getDesigner() const
{
    return Sanguosha->translate("designer:" + objectName(), "");
}

QString General::getIllustrator() const
{
    return Sanguosha->translate("illustrator:" + objectName(), "");
}

QString General::getCV() const
{
    return Sanguosha->translate("cv:" + objectName(), "");
}

QString General::getTinyIconPath() const
{
    QString path(getPixmapPath("tiny"));
    if(QFile::exists(path)){
        return path;
    }else{
        return "image/system/tiny_unknown.png";
    }
}

QSize General::BigIconSize(94, 96);
QSize General::SmallIconSize(122, 50);
QSize General::TinyIconSize(42, 36);
