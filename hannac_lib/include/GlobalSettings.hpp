#ifndef GLOBALSETTINGS_HPP
#define GLOBALSETTINGS_HPP

namespace hannac
{
class HSettings final
{
  public:
    static HSettings &get_settings()
    {
        static HSettings settings;
        return settings;
    }

    void set_verbose() noexcept
    {
        mVerbose = true;
    }
    bool get_verbose() const noexcept
    {
        return mVerbose;
    }

    HSettings(const HSettings &) = delete;
    HSettings &operator=(const HSettings &) = delete;

  private:
    HSettings() {};

    // Settings
    bool mVerbose = false;
};
} // namespace hannac
#endif