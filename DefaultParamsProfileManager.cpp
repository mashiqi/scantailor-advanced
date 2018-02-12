
#include <QtGui/QtGui>
#include <QtXml/QDomDocument>
#include "DefaultParamsProfileManager.h"
#include "DefaultParams.h"

using namespace page_split;
using namespace output;
using namespace page_layout;

DefaultParamsProfileManager::DefaultParamsProfileManager()
        : path(qApp->applicationDirPath() + "/config/profiles") {
}

DefaultParamsProfileManager::DefaultParamsProfileManager(const QString& path)
        : path(path) {
}

std::unique_ptr<std::list<QString>> DefaultParamsProfileManager::getProfileList() const {
    auto profileList = std::make_unique<std::list<QString>>();

    QDir dir(path);
    if (dir.exists()) {
        QList<QFileInfo> fileInfoList = dir.entryInfoList();
        for (const QFileInfo& fileInfo : fileInfoList) {
            if (fileInfo.isFile()
                && ((fileInfo.suffix() == "stp") || (fileInfo.suffix() == "xml"))) {
                profileList->push_back(fileInfo.baseName());
            }
        }
    }

    return profileList;
}

std::unique_ptr<DefaultParams> DefaultParamsProfileManager::readProfile(const QString& name) const {
    QDir dir(path);
    QFileInfo profile(dir.absoluteFilePath(name + ".stp"));
    if (!profile.exists()) {
        profile = dir.absoluteFilePath(name + ".xml");
        if (!profile.exists()) {
            return nullptr;
        }
    }

    QFile profileFile(profile.filePath());
    if (!profileFile.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    QDomDocument doc;
    if (!doc.setContent(&profileFile)) {
        return nullptr;
    }

    profileFile.close();

    return std::make_unique<DefaultParams>(doc.documentElement());
}

bool DefaultParamsProfileManager::writeProfile(const DefaultParams& params, const QString& name) const {
    QDomDocument doc;
    doc.appendChild(params.toXml(doc, "profile"));

    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(dir.absoluteFilePath(name + ".stp"));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream textStream(&file);
        doc.save(textStream, 2);
        return true;
    }

    return false;
}

std::unique_ptr<DefaultParams> DefaultParamsProfileManager::createDefaultProfile() const {
    return std::make_unique<DefaultParams>();
}

std::unique_ptr<DefaultParams> DefaultParamsProfileManager::createSourceProfile() const {
    DefaultParams::DeskewParams deskewParams;
    deskewParams.setMode(MODE_MANUAL);

    DefaultParams::PageSplitParams pageSplitParams;
    pageSplitParams.setLayoutType(SINGLE_PAGE_UNCUT);

    DefaultParams::SelectContentParams selectContentParams;
    selectContentParams.setContentDetectEnabled(false);

    DefaultParams::PageLayoutParams pageLayoutParams;
    pageLayoutParams.setHardMargins(Margins(0, 0, 0, 0));

    Alignment alignment;
    alignment.setNull(true);
    pageLayoutParams.setAlignment(alignment);

    DefaultParams::OutputParams outputParams;

    ColorParams colorParams;
    colorParams.setColorMode(COLOR_GRAYSCALE);

    ColorCommonOptions colorCommonOptions;
    colorCommonOptions.setCutMargins(false);
    colorParams.setColorCommonOptions(colorCommonOptions);

    outputParams.setColorParams(colorParams);

    return std::make_unique<DefaultParams>(
            DefaultParams::FixOrientationParams(),
            deskewParams,
            pageSplitParams,
            selectContentParams,
            pageLayoutParams,
            outputParams
    );
}

bool DefaultParamsProfileManager::deleteProfile(const QString& name) const {
    QDir dir(path);
    QFileInfo profile(dir.absoluteFilePath(name + ".stp"));
    if (!profile.exists()) {
        profile = dir.absoluteFilePath(name + ".xml");
        if (!profile.exists()) {
            return false;
        }
    }

    QFile profileFile(profile.filePath());
    return profileFile.remove();
}
