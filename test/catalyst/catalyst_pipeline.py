# script-version: 2.0
# Catalyst state generated using paraview version 5.11.2

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# ----------------------------------------------------------------
# setup views used in the visualization
# 

producer = TrivialProducer(registrationName="grid")

# # get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [620, 512]
renderView1.InteractionMode = '2D'
renderView1.AxesGrid = 'GridAxes3DActor'
renderView1.CenterOfRotation = [0.5, 0.5, 0.0]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [0.5, 0.5, 3.35]
renderView1.CameraFocalPoint = [0.5, 0.5, 0.0]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 0.7071067811865476
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(620, 512)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------



# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from disk_stretching_4_0000pvtu
disk_stretching_4_0000pvtuDisplay = Show(producer, renderView1, 'UnstructuredGridRepresentation')

# get 2D transfer function for 'u'
uTF2D = GetTransferFunction2D('u')
uTF2D.ScalarRangeInitialized = 1

# get color transfer function/color map for 'u'
uLUT = GetColorTransferFunction('u')
uLUT.AutomaticRescaleRangeMode = 'Never'
uLUT.TransferFunction2D = uTF2D
uLUT.ScalarRangeInitialized = 1.0

# get opacity transfer function/opacity map for 'u'
uPWF = GetOpacityTransferFunction('u')
uPWF.ScalarRangeInitialized = 1

# trace defaults for the display properties.
disk_stretching_4_0000pvtuDisplay.Representation = 'Surface'
disk_stretching_4_0000pvtuDisplay.ColorArrayName = ['POINTS', 'u']
disk_stretching_4_0000pvtuDisplay.LookupTable = uLUT
disk_stretching_4_0000pvtuDisplay.SelectTCoordArray = 'None'
disk_stretching_4_0000pvtuDisplay.SelectNormalArray = 'None'
disk_stretching_4_0000pvtuDisplay.SelectTangentArray = 'None'
disk_stretching_4_0000pvtuDisplay.OSPRayScaleArray = 'node-id'
disk_stretching_4_0000pvtuDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
disk_stretching_4_0000pvtuDisplay.SelectOrientationVectors = 'None'
disk_stretching_4_0000pvtuDisplay.ScaleFactor = 0.1
disk_stretching_4_0000pvtuDisplay.SelectScaleArray = 'None'
disk_stretching_4_0000pvtuDisplay.GlyphType = 'Arrow'
disk_stretching_4_0000pvtuDisplay.GlyphTableIndexArray = 'None'
disk_stretching_4_0000pvtuDisplay.GaussianRadius = 0.005
disk_stretching_4_0000pvtuDisplay.SetScaleArray = ['POINTS', 'node-id']
disk_stretching_4_0000pvtuDisplay.ScaleTransferFunction = 'PiecewiseFunction'
disk_stretching_4_0000pvtuDisplay.OpacityArray = ['POINTS', 'node-id']
disk_stretching_4_0000pvtuDisplay.OpacityTransferFunction = 'PiecewiseFunction'
disk_stretching_4_0000pvtuDisplay.DataAxesGrid = 'GridAxesRepresentation'
disk_stretching_4_0000pvtuDisplay.PolarAxes = 'PolarAxesRepresentation'
disk_stretching_4_0000pvtuDisplay.ScalarOpacityFunction = uPWF
disk_stretching_4_0000pvtuDisplay.ScalarOpacityUnitDistance = 0.03687610389237821
disk_stretching_4_0000pvtuDisplay.OpacityArrayName = ['POINTS', 'node-id']
disk_stretching_4_0000pvtuDisplay.SelectInputVectors = [None, '']
disk_stretching_4_0000pvtuDisplay.WriteLog = ''
disk_stretching_4_0000pvtuDisplay.custom_kernel = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
disk_stretching_4_0000pvtuDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 56802.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
disk_stretching_4_0000pvtuDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 56802.0, 1.0, 0.5, 0.0]

# setup the color legend parameters for each legend in this view

# get color legend/bar for uLUT in view renderView1
uLUTColorBar = GetScalarBar(uLUT, renderView1)
uLUTColorBar.Orientation = 'Horizontal'
uLUTColorBar.WindowLocation = 'Any Location'
uLUTColorBar.Position = [0.1651612903225807, 0.0]
uLUTColorBar.Title = 'u'
uLUTColorBar.ComponentTitle = ''
uLUTColorBar.ScalarBarLength = 0.7719354838709691

# set color bar visibility
uLUTColorBar.Visibility = 1

# show color legend
disk_stretching_4_0000pvtuDisplay.SetScalarBarVisibility(renderView1, True)

# ----------------------------------------------------------------
# setup color maps and opacity mapes used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup extractors
# ----------------------------------------------------------------

# create extractor
pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
# trace defaults for the extractor.
pNG1.Trigger = 'TimeStep'

# init the 'PNG' selected for 'Writer'
pNG1.Writer.FileName = 'RenderView1_{timestep:06d}{camera}.png'
pNG1.Writer.ImageResolution = [620, 512]
pNG1.Writer.Format = 'PNG'

# ----------------------------------------------------------------
# restore active source
SetActiveSource(pNG1)
# ----------------------------------------------------------------

# ------------------------------------------------------------------------------
# Catalyst options
from paraview import catalyst
options = catalyst.Options()
options.GlobalTrigger = 'TimeStep'
options.CatalystLiveTrigger = 'TimeStep'

print("executing catalyst_pipeline")

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
