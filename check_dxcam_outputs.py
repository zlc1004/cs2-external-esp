import dxcam

try:
    # Use dxcam.get_output_metadata() to get information about all connected displays
    output_metadata = dxcam.get_output_metadata()
    if output_metadata:
        print("DXCam Output Metadata:")
        for i, metadata in enumerate(output_metadata):
            print(f"Output Index {i}:")
            print(f"  Resolution: {metadata['resolution']}")
            print(f"  Position: {metadata['position']}")
            print(f"  Is Primary: {metadata['is_primary']}")
            print("---")
    else:
        print("No DXCam output metadata found.")
except Exception as e:
    print(f"An error occurred while getting DXCam output metadata: {e}")
