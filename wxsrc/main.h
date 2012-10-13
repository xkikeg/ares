#include <wx/app.h>
#include <wx/intl.h>
#include <memory>

namespace ares
{
  class CDatabase;
}

class IncrementFareHandler;

class AresApp : public ::wxApp
{
public:
  //! コンストラクタ
  AresApp();
  //! セットアップ時の処理
  virtual bool OnInit();

  const ares::CDatabase & getdb() const;

protected:
  wxLocale m_locale;

private:
  std::shared_ptr<ares::CDatabase> m_aresdb;
  std::unique_ptr<IncrementFareHandler> m_pIncrementFareHandler;
  DECLARE_NO_COPY_CLASS(AresApp);
};

DECLARE_APP(AresApp);
