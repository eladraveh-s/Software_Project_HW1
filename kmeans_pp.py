import mykmeanssp as k

def strToFloat(p):
    for i in range(len(p)):
        p[i] = float(p[i])
    return p

def listOfpoints(input_data):
    dataPoints = []
    inputData = open(input_data)
    while(True):
        pLine = inputData.readline()
        strPoint = pLine.split(',')
        if strPoint == [""]:
            break
        point = strToFloat(strPoint)
        dataPoints.append(point)

    return dataPoints

def main(file):
    centroids = [[-0.9626,7.5403,-0.5724,-3.6368,-4.5917,10.0681,-5.6816,-5.8396,6.0236,-0.7875,-2.5990], [-0.4067,7.2956,-0.7195,-4.5495,-1.9023,9.1612,-5.7507,-4.8608,5.5236,-3.2573,-1.8133], [-1.9321,3.4040,-1.6279,-9.9915,-7.1524,-0.6751,-9.3389,-3.4245,8.3276,-3.8222,-3.7307],[-5.0485,10.4544,5.5042,10.5971,-4.3470,-9.1527,7.7245,-7.8383,-9.8023,0.7996,-8.0826],[-2.3281,4.2471,-0.8386,-7.7549,-7.1890,-1.3090,-8.9342,-3.8278,8.4505,-4.0044,-4.0869],[1.7999,1.1123,4.7557,7.1323,-8.9596,-2.6029,-0.7020,2.2333,2.8796,-1.8032,-3.5543],[-0.1851,-0.2895,5.8976,7.0085,-8.8534,-3.1448,-2.1227,4.7639,0.5728,1.3704,-3.5704]]
    dataPoints = listOfpoints(file)
    a = k.fit(200, 0.001, centroids, dataPoints)
    print(a)

if __name__ == '__main__':
    main('input2.txt')
