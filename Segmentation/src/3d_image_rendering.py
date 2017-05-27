import bpy
import math
import mathutils
import parser

#This is the original function
def _adjust_camera(camera_coordinates, target_coordinates, scene):
    scene.camera.location.x = camera_coordinates[0]
    scene.camera.location.y = camera_coordinates[1]
    scene.camera.location.z = camera_coordinates[2]

    # Defining dx, dy and dz for calculations
    dx = target_coordinates[0] - camera_coordinates[0]
    dy = target_coordinates[1] - camera_coordinates[1]
    dz = target_coordinates[2] - camera_coordinates[2]

    dx = target_coordinates[0]
    dy = target_coordinates[1]
    dz = target_coordinates[2]
	
    print("dx, dy, dz:", dx, dy, dz)

    # Calculating euler rotation xrad and zrad
    xRad = (3.14159/2.) + math.atan2(dz, math.sqrt(dy**2 + dx**2))
    print("xRad: %f, %f deg" % (xRad, xRad*180./math.pi))

    zRad = math.atan2(dy, dx) - (3.14159256 / 2.)
    print("zRad: %f, %f deg" % (zRad, zRad*180./math.pi))
# print("xRad: %f, zRad: %f " % (xRad, zRad))

    # Applying camera rotation to scene
    scene.camera.rotation_euler = mathutils.Euler((xRad, 0, zRad), 'XYZ')

    print("Camera adjusted")



def adjust_camera(camera_coordinates, eulers, scene):

    scene.camera.location.x = camera_coordinates[0]
    scene.camera.location.y = camera_coordinates[1]
    scene.camera.location.z = camera_coordinates[2]

    # Applying camera rotation to scene
    scene.camera.rotation_mode = "XYZ"
    scene.camera.rotation_euler = eulers

    print("Camera adjusted")


def capture_image(scene, path):

    # Render image to given path
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    print("Image captured")


def build_array(string):
	#This is where I should be parsing the format of the matrix into correct format, like bytearray
#	[0.9996112060025656, 0.02751455566943147, 0.004515092514032283, 30.88144708139786;
# -0.02727902114228923, 0.9985763003920589, -0.04583914594347553, 92.68978101851303;
# -0.005769908111491143, 0.04569815665453551, 0.9989386300663132, 1793.480325390813;
# 0, 0, 0, 1]

	cam_coords[0] = 10
	cam_coords[1] = 10
	cam_coords[2] = 10


def main_(string):

    if(string):
        print(string.decode("ascii"))

    # Open .blender file
    # Filepath for 3D .blend model
    bpy.ops.wm.open_mainfile(filepath='../../pics/stadium.blend')

    # Create scene
    scene = bpy.data.scenes["Scene"]
    # Camera coordinates for testing
    camera_location = [30.0, 30.0, 30.0]
    # Target location for testing
    target_location = [0, 0, 0]
    # Path to save rendered image in
    image_path = 'render.jpg'

    adjust_camera(camera_location, euler_angles, scene)
    capture_image(scene, image_path)
    print("Python finished\n\n\n")



if __name__ == "__main__":

    # Open .blender file
    # Filepath for 3D .blend model
#  bpy.ops.wm.open_mainfile(filepath='../../pics/stadium.blend')
    bpy.ops.wm.open_mainfile(filepath='/home/jyri/Desktop/BlenderApi/land.blend')

    # Create scene
    scene = bpy.data.scenes["Scene"]
    # Camera coordinates for testing
    camera_location = [-0.18, -17.9, 1.75]
    # Target location for testing [-0.00576991  0.04569816  0.99893863]

    target_location = [-0.00576991,    0.99893863, 0.04569816]
    # Path to save rendered image in
    image_path = 'render.jpg'

    euler_angles = [ 1.52508149,  0.00576994, -0.02728286]
    adjust_camera(camera_location, euler_angles, scene)
#_adjust_camera(camera_location, target_location, scene)
    capture_image(scene, image_path)

    print("Python finished\n\n")
