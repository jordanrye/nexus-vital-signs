#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>

namespace Settings {
    
    void Load(const std::filesystem::path& aSettingsPath);
    void Save(const std::filesystem::path& aSettingsPath);

    void LoadLayout(const std::filesystem::path& layoutFilePath);
    void SaveLayout(const std::filesystem::path& layoutFilePath);

} // namespace Settings

#endif /* SETTINGS_H */