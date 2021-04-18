from diagrams import Diagram, Cluster, Edge
from diagrams.aws.iot import IotDeviceGateway
from diagrams.aws.iot import IotActuator
from diagrams.aws.devtools import CloudDevelopmentKit
from diagrams.aws.compute import Compute
from diagrams.aws.database import Dynamodb
from diagrams.aws.analytics import Redshift
from diagrams.aws.analytics import Analytics
from diagrams.aws.compute import Lambda
from diagrams.generic.device import Tablet
from diagrams.aws.iot import IotDeviceManagement

with Diagram("ATM IoT architecture", show=False):
    with Cluster("IoT enabling technology"):
        gateway1 = IotDeviceGateway("Gateway")
        gateway2 = IotDeviceGateway("Gateway")
        gateway1 - Compute("Sensors & actuators")
        gateway2 - Compute("Sensors & actuators")
    with Cluster("IoT software platform"):
        cloud_gateway = CloudDevelopmentKit("Cloud gateway")
        stream_data = Dynamodb("Data processor")
        data_warehouse = Redshift("Big data warehouse")
        dev_manage = IotDeviceManagement("Control applications")
        cloud_gateway >> stream_data >> Edge(label = "Sensor data") >> data_warehouse
        stream_data >> Edge(label = "Sensor data") >> dev_manage
        data_warehouse >> dev_manage
        dev_manage >> Edge(label = "Control data") >> cloud_gateway
    with Cluster("IoT applications"):
        data_analytics = Analytics("Data analytics")
        business_logic = Lambda("User business logic")
        web_app = Tablet("Web app")
        business_logic >> web_app

    gateway1 >> cloud_gateway
    cloud_gateway >> gateway2
    data_warehouse >> data_analytics
    data_warehouse >> business_logic
