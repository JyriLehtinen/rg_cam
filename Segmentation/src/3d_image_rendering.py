import bpy
import math
import mathutils


def adjust_camera(camera_coordinates, target_coordinates, scene):
    scene.camera.location.x = camera_coordinates[0]
    scene.camera.location.y = camera_coordinates[1]
    scene.camera.location.z = camera_coordinates[2]

    # Defining dx, dy and dz for calculations
    dx = target_coordinates[0] - camera_coordinates[0]
    dy = target_coordinates[1] - camera_coordinates[1]
    dz = target_coordinates[2] - camera_coordinates[2]

    print("dx, dy, dz:", dx, dy, dz)

    # Calculating euler rotation xrad and zrad
    xRad = (3.14159/2.) + math.atan2(dz, math.sqrt(dy**2 + dx**2))
    print("xRad: %f, %f deg" % (xRad, xRad*180./math.pi))

    zRad = math.atan2(dy, dx) - (3.14159256 / 2.)
    print("zRad: %f, %f deg" % (zRad, zRad*180./math.pi))

    # Applying camera rotation to scene
    scene.camera.rotation_euler = mathutils.Euler((xRad, 0, zRad), 'XYZ')

    print("Camera adjusted")


def capture_image(scene, path):

    # Render image to given path
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    print("Image captured")


def main_(string):
###if __name__ == "__main__":

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

    adjust_camera(camera_location, target_location, scene)
#capture_image(scene, image_path)
    print("Python finished\n\n\n\n")
