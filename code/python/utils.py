import io
import uuid
import numpy as np
import laspy

from PIL import Image
from pye57 import libe57
from pye57.libe57 import VectorNode, ImageFile

def get_point_cloud(file_path: str) -> dict:
    # This function gets point cloud from LAS file

    las_file = laspy.open(file_path)
    data = las_file.read()
    x_data = np.asarray(data.x).astype(np.float64)
    y_data = np.asarray(data.y).astype(np.float64)
    z_data = np.asarray(data.z).astype(np.float64)
    color_red = np.asarray(data.red).astype(np.float64)
    color_green = np.asarray(data.green).astype(np.float64)
    color_blue = np.asarray(data.blue).astype(np.float64)

    max_red = np.max(color_red)
    max_green = np.max(color_green)
    max_blue = np.max(color_blue)
    do_rescale = max_red > 255 or max_green > 255 or max_blue > 255
    if do_rescale:
        color_red /= 255
        color_green /= 255
        color_blue /= 255

    point_cloud = {
        "cartesianX": x_data,
        "cartesianY": y_data,
        "cartesianZ": z_data,
        "colorRed": color_red,
        "colorGreen": color_green,
        "colorBlue": color_blue,
    }
    return point_cloud

def generate_random_point_cloud(num_points: int=1000, coord_range: tuple=(0, 100), color_range: tuple=(0, 255)) -> dict:
    # This function generate random 3D coordinates (x, y, z)

    min_coord, max_coord = coord_range
    min_color, max_color = color_range

    # Generate random 3D coordinates (x, y, z) within the specified range
    _cartesianX = np.random.uniform(min_coord, max_coord, num_points).astype(float)
    _cartesianY = np.random.uniform(min_coord, max_coord, num_points).astype(float)
    _cartesianZ = np.random.uniform(min_coord, max_coord, num_points).astype(float)

    # Generate random color components (r, g, b)
    _colorRed = np.random.uniform(min_color, max_color, num_points).astype(np.uint8)
    _colorGreen = np.random.uniform(min_color, max_color, num_points).astype(np.uint8)
    _colorBlue = np.random.uniform(min_color, max_color, num_points).astype(np.uint8)

    # Create the point cloud dictionary
    point_cloud = {
        "cartesianX": _cartesianX,
        "cartesianY": _cartesianY,
        "cartesianZ": _cartesianZ,
        "colorRed": _colorRed,
        "colorGreen": _colorGreen,
        "colorBlue": _colorBlue
    }

    return point_cloud


def write_image_e57(
        images2d: VectorNode,
        image_file: ImageFile,
        image: Image,
        translation: np.ndarray = np.asarray([0.0, 0.0, 0.0]),
        rotation : np.ndarray = np.asarray([1.0, 0.0, 0.0, 0.0]),
        pixel_height: float = 0.001,
        pixel_width: float = 0.001,
        image_name: str = "Image",
        description: str = "My awesome image",
        source: str = "You scanner / software name",
) -> None:
    # This function writes a spherical pano image to an existing E57 file

    # Create a node for the image to be added
    image_node = libe57.StructureNode(image_file)
    image_node.set("guid", libe57.StringNode(image_file, "{%s}" % uuid.uuid4()))
    image_node.set("name", libe57.StringNode(image_file, "{%s}" % image_name))
    image_node.set("description", libe57.StringNode(image_file, "{%s}" % description))
    image_node.set("sensorVendor", libe57.StringNode(image_file, "{%s}" % source))

    # Specify the position and rotation of the pano image
    image_pose_node = libe57.StructureNode(image_file)
    image_node.set("pose", image_pose_node)

    # Specify the position of the image in meters
    rotation_node = libe57.StructureNode(image_file)
    rotation_node.set("w", libe57.FloatNode(image_file, rotation[0]))
    rotation_node.set("x", libe57.FloatNode(image_file, rotation[1]))
    rotation_node.set("y", libe57.FloatNode(image_file, rotation[2]))
    rotation_node.set("z", libe57.FloatNode(image_file, rotation[3]))
    image_pose_node.set("rotation", rotation_node)

    # Specify the rotation quaternion of the image
    translation_node = libe57.StructureNode(image_file)
    translation_node.set("x", libe57.FloatNode(image_file, translation[0]))
    translation_node.set("y", libe57.FloatNode(image_file, translation[1]))
    translation_node.set("z", libe57.FloatNode(image_file, translation[2]))
    image_pose_node.set("translation", translation_node)

    # Specify the dimensions of the image
    image_representation_node = libe57.StructureNode(image_file)
    image_node.set("sphericalRepresentation", image_representation_node)
    image_representation_node.set("imageHeight", libe57.IntegerNode(image_file, image.height))
    image_representation_node.set("imageWidth", libe57.IntegerNode(image_file, image.width))
    image_representation_node.set("pixelHeight", libe57.FloatNode(image_file, pixel_height))
    image_representation_node.set("pixelWidth", libe57.FloatNode(image_file, pixel_width))

    # Save the image to E57 file
    blob = io.BytesIO()
    image.save(blob, format='JPEG')
    blob_data = blob.getvalue()
    blob_node = libe57.BlobNode(image_file, len(blob_data))
    image_representation_node.set("jpegImage", blob_node)
    images2d.append(image_node)
    blob_node.write(blob_data, 0, len(blob_data))
