#include <wx/app.h>
#include <wx/intl.h>
#include <memory>

namespace ares
{
  class CDatabase;
}

class AresApp : public ::wxApp
{
public:
  //! コンストラクタ
  AresApp() {};
  //! セットアップ時の処理
  virtual bool OnInit();

  const ares::CDatabase & getdb() const;

protected:
  wxLocale m_locale;

private:
  std::shared_ptr<ares::CDatabase> m_aresdb;
  DECLARE_NO_COPY_CLASS(AresApp);
};

DECLARE_APP(AresApp);
