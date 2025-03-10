/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "geometryengine.h"
#include "geometryengine.h"

#include <QVector2D>
#include <QVector3D>


struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

//! [0]
GeometryEngine::GeometryEngine()
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    this->filename = "woman.obj";
    //this->filename = "man.obj";
    initCubeGeometry();

}

GeometryEngine::~GeometryEngine()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}
//! [0]



void GeometryEngine::readfile(string filename)
{
    QString myfile = QString::fromStdString(filename);

    QFile file(myfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;


    vetexes.clear();
    faces.clear();
    textures.clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList l2 = line.split(' ', QString::KeepEmptyParts);

        if (l2[0]== "v"){
            vertex v;
            v.x = l2[1].toFloat();
            v.y = l2[2].toFloat();
            v.z = l2[3].toFloat();

            this->vetexes.push_back(v);

        }else if (l2[0]=="vt"){
            texture t;
            t.x = l2[1].toFloat();
            t.y = l2[2].toFloat();
            //qDebug()<<t.x<<t.y<<endl;
            this->textures.push_back(t);
        }
        else if ( l2[0]=="f"){

            QStringList l2_1 = l2[1].split('/', QString::KeepEmptyParts);
            QStringList l2_2 = l2[2].split('/', QString::KeepEmptyParts);
            QStringList l2_3 = l2[3].split('/', QString::KeepEmptyParts);

            face f;
            f.v1.v = l2_1[0].toFloat();f.v1.t = l2_1[1].toFloat();
            f.v2.v = l2_2[0].toFloat();f.v2.t = l2_2[1].toFloat();
            f.v3.v = l2_3[0].toFloat();f.v3.t = l2_3[1].toFloat();

            faces.push_back(f);
        }
    }
    file.close();
}

void GeometryEngine::initCubeGeometry()
{
    // For cube we would need only 8 vertices but we have to
    // duplicate vertex for each face because texture coordinate
    // is different.

    readfile(filename);

    this->vertices_tam = this->faces.size()*3;
    VertexData vertices[vertices_tam];

    this->indices_tam = (this->faces.size()*3)+(this->faces.size()*2);
    GLushort indices[indices_tam];

    int pos_vertices=0;
    //fill indices
    int v_vertex=-1;
    int v_texture=-1;
    int pos_ind=0;

    for(int i=0; i<this->faces.size(); i++){

        vertex v1 = vetexes[faces.at(i).v1.v-1];texture t1 = textures[faces.at(i).v1.t-1];
        vertices[pos_vertices] = {QVector3D(v1.x,v1.y,v1.z), QVector2D(t1.x, t1.y)};pos_vertices++;

        vertex v2 = vetexes[faces.at(i).v2.v-1];texture t2 = textures[faces.at(i).v2.t-1];
        vertices[pos_vertices] = {QVector3D(v2.x,v2.y,v2.z), QVector2D(t2.x, t2.y)};pos_vertices++;

        vertex v3 = vetexes[faces.at(i).v3.v-1];texture t3 = textures[faces.at(i).v3.t-1];
        vertices[pos_vertices] = {QVector3D(v3.x,v3.y,v3.z), QVector2D(t3.x, t3.y)};pos_vertices++;

        //indices
        indices[pos_ind] = v_vertex++; pos_ind++;
        indices[pos_ind] = v_vertex++; pos_ind++;
        indices[pos_ind] = v_vertex++; pos_ind++;

        indices[pos_ind] = v_texture++; pos_ind++;
        indices[pos_ind] = v_texture++; pos_ind++;
    }

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, this->vertices_tam * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, this->indices_tam * sizeof(GLushort));
}

//! [2]
void GeometryEngine::drawCubeGeometry(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawArrays( GL_TRIANGLES, 0, this->vertices_tam );

    program->disableAttributeArray( vertexLocation );
    program->disableAttributeArray( texcoordLocation );

}
//! [2]
