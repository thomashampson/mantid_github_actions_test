#ifndef MANTID_MANTIDWIDGETS_WORKSPACEPRESENTER_H_
#define MANTID_MANTIDWIDGETS_WORKSPACEPRESENTER_H_

#include "MantidQtMantidWidgets/WorkspacePresenter/ViewNotifiable.h"
#include "MantidQtMantidWidgets/WorkspacePresenter/WorkspaceProviderNotifiable.h"
#include <boost/weak_ptr.hpp>
#include <memory>

namespace MantidQt {
namespace MantidWidgets {

class IWorkspaceDockView;
class WorkspaceProvider;

using DockView_sptr = boost::shared_ptr<IWorkspaceDockView>;
using DockView_wptr = boost::weak_ptr<IWorkspaceDockView>;
using ADSAdapter_uptr = std::unique_ptr<WorkspaceProvider>;
/**
\class  WorkspacePresenter
\brief  Presenter class for Workspace dock in MantidPlot UI
\author Lamar Moore
\date   24-08-2016
\version 1.0


Copyright &copy; 2016 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
National Laboratory & European Spallation Source

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>
*/
class WorkspacePresenter : public WorkspaceProviderNotifiable,
                           public ViewNotifiable {
public:
  explicit WorkspacePresenter(DockView_wptr view);
  ~WorkspacePresenter() = default;

  void
  notifyFromWorkspaceProvider(WorkspaceProviderNotifiable::Flag flag) override;
  void notifyFromView(ViewNotifiable::Flag flag) override;

private:
  void loadWorkspace();
  void saveWorkspace();
  void renameWorkspace();
  void groupWorkspaces();
  void sortWorkspaces();
  void deleteWorkspaces();

  void workspaceLoaded();
  void workspaceSaved();
  void workspaceRenamed();
  void workspacesGrouped();
  void workspacesSorted();
  void workspacesDeleted();

private:
  DockView_wptr m_view;
  ADSAdapter_uptr m_adapter;
};
} // namespace MantidWidgets
} // namespace MantidQt
#endif // MANTID_MANTIDWIDGETS_WORKSPACEPRESENTER_H_