 
import os
import subprocess
import bpy
#import mathutils

from bpy.props import (StringProperty,
                       BoolProperty,
                       IntProperty,
                       FloatProperty,
                       FloatVectorProperty,
                       EnumProperty,
                       PointerProperty,
                       )
from bpy.types import (Panel,
                       Operator,
                       AddonPreferences,
                       PropertyGroup,
                       )

bl_info = {
    "name": "RayTracing",
    "description": "Lauch Ray Tracing calculation for RIR generation and display results",
    "author": "Robin GUEGUEN",
    "version": (0, 0),
    "blender": (2, 77, 0),
    "warning": "",
    "category": "3D View"}

#def command(program, args, flags, values):
#    commandString = [program]
#    for a in args:
#        commandString.append(str(a))
#    for f,v in zip(flags, values):
#        st = "-"
#        st += str(f)
#        if v != None:
#            st += " " + str(v)
#        commandString.append(st)
#    st = ""
#    for c in commandString:
#        st += c + " "
#    print(st)
#    os.system(st)

#def MMG(TOOL):
#    mesh = TOOL.file#[:-5]#+".tmp.mesh"
#    sol = mesh[:-5] + ".sol"
#    out = mesh
#    args = [mesh, out]
#    flags =  ["hgrad",    "hausd",    "hmin",    "hmax"]
#    hmin = TOOL.hmin/100.0 * max(bpy.context.object.dimensions)  
#    hmax = TOOL.hmax/100.0 * max(bpy.context.object.dimensions)
#    hausd = TOOL.hausd/100.0 * max(bpy.context.object.dimensions)
#    values = [TOOL.hgrad, hausd, hmin, hmax]
#    if not TOOL.nr:
#        flags.append("nr")
#        values.append(None)
#    flags.append("sol")
#    values.append(sol)
#    command("mmgs_O3", args, flags, values)
#    return out

## Open executable file
def OPEN(file):
    #os.system(file)
    subprocess.Popen(file)


class Settings(PropertyGroup):
#    medit = BoolProperty(name="medit",
#                         description="Display in MEDIT",
#                         default = True)
#    #sol = BoolProperty(name="sol",
#    #                     description="Use .sol file if available",
#    #                     default = True)
#    nr = BoolProperty(name="angle",
#                      description="Angle Detection",
#                      default = True)
#    reload = BoolProperty(name="reload",
#                          description="Load remeshed file",
#                          default = False)

#    hmin = FloatProperty(name = "hmin (%)",
#                         description="Minimal edge length (%)",
#                         default = 0.5,
#                         min = 0.01,
#                         max = 20
#    )
#    hmax = FloatProperty(name = "hmax (%)",
#                         description="Maximal edge length (%)",
#                         default = 5,
#                         #min = 0.1,
#                         min = 0.03,
#                         max = 100
#    )
#    hausd = FloatProperty(name = "hausd (%)",
#                         description="Haussdorf distance (%)",
#                         default = 2,
#                         min = 0.001,
#                         max = 20
#    )
#    hgrad = FloatProperty(name = "hgrad",
#                         description="Gradation",
#                         default = 1.08,
#                         min = 1,
#                         max = 10
#    )
    file = StringProperty(name = "file",
                          default = "",
                          description = "File to be lauched",
                          subtype = 'FILE_PATH'
    )
    file2 = StringProperty(name = "file2",
                          default = "",
                          description = "File to be imported",
                          subtype = 'FILE_PATH'
    )    

class RayTracingPanel(bpy.types.Panel):
    bl_label = "Ray Tracing"
    #bl_idname = "OBJECT_PT_hello"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    #bl_context = "object"
    bl_category = "Tools"
    
    def draw(self, context):
        mytool = context.scene.my_tool
        
        TheCol = self.layout.column(align=True)
        TheCol.operator("bpy.ops.mesh.primitive_uv_sphere_add", text="New Source", icon="MESH_UVSPHERE")
        
        # FILE
        self.layout.prop(mytool, "file", text="App file")
        
        # export buttons
        TheCol = self.layout.column(align=True)
        
        row = TheCol.row(align=True)
        row.operator("exp.src", text="EXPORT SOURCE", icon="MESH_UVSPHERE")
        row.operator("exp.listener", text="EXPORT LISTENER", icon="MESH_MONKEY")
        # run button
        TheCol = self.layout.column(align=True)
        TheCol.operator("run.rt", text="EXPORT MESH & RUN", icon="PLAY")
                
        #import
        self.layout.prop(mytool, "file2", text="Import Obj")
        
        TheCol = self.layout.column(align=True)
        TheCol.operator("imp.rt", text="IMPORT", icon="IMPORT")

class runRayTracing(bpy.types.Operator):
    bl_idname = "run.rt"
    bl_label = "Lauch Ray Tracing algorytme"

    def invoke(self, context, event):
        T = context.scene.my_tool
        inFile = T.file # recupération du répertoire de l'app
        objFile = T.file[:-13] + "meshForRayTracing.obj"
        #print ("le repertoire est :" + folder)
        try:            
            #bpy.ops.export_scene.obj(filepath = objFile,use_selection=True,use_mesh_modifiers=True,use_triangles=True, axis_forward='Y', axis_up='Z')
            bpy.ops.export_scene.obj(filepath = objFile,use_selection=True,use_mesh_modifiers=True,use_triangles=True)
            #bpy.ops.export_mesh.mesh(filepath = inFile)           

##### Attention le chemin du repertoire doit être complet #######
            
            OPEN(inFile)

        except AttributeError:
            print("OUPS !")
        return {"FINISHED"}
    
class expSource(bpy.types.Operator):
    bl_idname = "exp.src"
    bl_label = "Export Source"

    def invoke(self, context, event):
        T = context.scene.my_tool
        inFile = T.file # recupération du répertoire de l'app
        srcFile = T.file[:-13] + "srcForRayTracing.obj"
        #print ("le repertoire est :" + folder)
        try:            
            bpy.ops.export_scene.obj(filepath = srcFile,use_selection=True,use_mesh_modifiers=True,use_triangles=True)          

        except AttributeError:
            print("OUPS !")
        return {"FINISHED"}
    
class expListener(bpy.types.Operator):
    bl_idname = "exp.listener"
    bl_label = "Export Listener"

    def invoke(self, context, event):
        T = context.scene.my_tool
        inFile = T.file # recupération du répertoire de l'app
        ltnFile = T.file[:-13] + "listenerForRayTracing.obj"
        #print ("le repertoire est :" + folder)
        try:            
            bpy.ops.export_scene.obj(filepath = ltnFile,use_selection=True,use_mesh_modifiers=True,use_triangles=True)          

        except AttributeError:
            print("OUPS !")
        return {"FINISHED"}

class impRayTracing(bpy.types.Operator):
    bl_idname = "imp.rt"
    bl_label = "Import raytracing algorythme Output"

    def invoke(self, context, event):
        T = context.scene.my_tool
        
        inFile = T.file2 # recupération du répertoire de l'app
#        objFile = T.file[:-13] + "meshForRayTracing.obj"
#        #print ("le repertoire est :" + folder)
        try:            
#            #bpy.ops.export_scene.obj(filepath = objFile,use_selection=True,use_mesh_modifiers=True,use_triangles=True, axis_forward='Y', axis_up='Z')
             bpy.ops.import_scene.obj(filepath = inFile)
             

        except AttributeError:
            print("OUPS !")
        return {"FINISHED"}

bpy.utils.register_class(expListener)
bpy.utils.register_class(expSource)
bpy.utils.register_class(impRayTracing)
bpy.utils.register_class(runRayTracing)
bpy.utils.register_class(RayTracingPanel)

def register():
    bpy.utils.register_module(__name__)
    bpy.types.Scene.my_tool = PointerProperty(type=Settings)

def unregister():
    bpy.utils.unregister_module(__name__)
    del bpy.types.Scene.my_tool

if __name__ == "__main__":
    register()
