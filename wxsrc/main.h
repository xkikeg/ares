#include <wx/app.h>
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

  std::shared_ptr<ares::CDatabase> getdb() const;

private:
  std::shared_ptr<ares::CDatabase> m_aresdb;
  DECLARE_NO_COPY_CLASS(AresApp);
};

DECLARE_APP(AresApp);
