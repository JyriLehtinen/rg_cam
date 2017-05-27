import numpy
import math

def swap_cols(arr, frm, to):
    arr[:,[frm, to]] = arr[:,[to, frm]]

def swap_rows(arr, frm, to):
    arr[[frm, to],:] = arr[[to, frm],:]

def rotationMatrixToEulerAngles(R) :
 
    sy = math.sqrt(R[0,0] * R[0,0] +  R[1,0] * R[1,0])
     
    singular = sy < 1e-6
 
    if  not singular :
        x = math.atan2(R[2,1] , R[2,2])
        y = math.atan2(-R[2,0], sy)
        z = math.atan2(R[1,0], R[0,0])
    else :
        x = math.atan2(-R[1,2], R[1,1])
        y = math.atan2(-R[2,0], sy)
        z = 0
 
    return numpy.array([x, y, z])



if __name__ == "__main__" :

	print("\n\n\n\n\n\t\t\t***RUNNING PARSING TEST***\n\n\n")

	test_data = "[0.9996112060025656, 0.02751455566943147, 0.004515092514032283, 30.88144708139786;\n  -0.02727902114228923, 0.9985763003920589, -0.04583914594347553, 92.68978101851303;\n  -0.005769908111491143, 0.04569815665453551, 0.9989386300663132, 1793.480325390813;\n  0, 0, 0, 1]"

	print(test_data)
	
#remove the newline characters
	test_data = test_data.replace('\n', "")

#construct the float matrix from the string
	Matrix = numpy.matrix(test_data)

	print(Matrix)

	translation = Matrix[:, 3]
	translation = translation[:3]
#	translation = numpy.squeeze(numpy.asarray(translation))

	print("The translation vector is:")
	print(translation)

	rotation = Matrix[0:3,0:3]
	swap_cols(rotation, 1, 2)
	print("\nRotation matrix:")
	print(rotation)
	print("\n\n")

#Linear algebra: 0 = RC + T ==> C = -R^t * T
	position = -(rotation.transpose() * translation)
	position = numpy.squeeze(numpy.asarray(position))
	position[2] = -position[2]

	temp_vector = [0, 0, 1]
	direction = rotation.transpose() * numpy.matrix(temp_vector).transpose()
	direction = numpy.squeeze(numpy.asarray(direction))

#Taking negatives because Blender and OpenCV use different coordinates
#	position[0] = position[0]/100.0
#	temp = position[1]
#	position[1] = -position[2]/100.0
#	position[2] = -temp/100.0
	

	print("\n\n\nCamera coordinates in m!")
	print(position)
	print("\nDirection:")
	print(direction)

#Converting the rotation matrix to euler angles and printing it
	print("\n\nRotation euler angles")
	print(rotationMatrixToEulerAngles(rotation))

