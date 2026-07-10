#include "article_injections.hh"
#include "config.hh"
#include <QString>

std::vector< Injection > articleInjections( const Config::Preferences & cfg )
{
  std::vector< Injection > result;

  if ( cfg.entryHeightLimit ) {
    // overflow-y:hidden provides the BFC to prevent the ghost scroll layer
    // Chromium sometimes allocates with overflow-y:auto even when content
    // fits. JS switches to overflow-y:auto when content genuinely exceeds
    // the cap (>2px tolerance for sub-pixel rounding). FORK-SPEC #13.
    Injection inj;
    inj.tag = QString( R"(<style>.gdarticlebody{overflow-y:hidden;}:root{--gd-entry-height:%1px;}.gdarticlebody{max-height:calc(var(--gd-entry-height) + 16px);}</style>)" )
                .arg( QString::number( cfg.entryMaxHeight ) )
                .toStdString();
    result.push_back( inj );

    inj.tag = R"(<script>
(function(){
function fix(){var els=document.querySelectorAll('.gdarticlebody'),i;
for(i=0;i<els.length;i++){var e=els[i];
e.style.overflowY=e.scrollHeight-e.clientHeight>2?'auto':'';}}
requestAnimationFrame(function(){requestAnimationFrame(function(){
fix();
window.addEventListener('load',fix);
if(document.fonts&&document.fonts.ready)document.fonts.ready.then(fix);
});});
})();
</script>)";
    result.push_back( inj );
  }

  return result;
}
