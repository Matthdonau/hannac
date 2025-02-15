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

    void set_verbose(int lev) noexcept
    {
        mVerbose = lev;
    }
    int get_verbose() const noexcept
    {
        return mVerbose;
    }

    HSettings(const HSettings &) = delete;
    HSettings &operator=(const HSettings &) = delete;

  private:
    HSettings() {};

    // Settings
    int mVerbose = 0;
};
} // namespace hannac
#endif