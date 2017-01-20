#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utilities.h"
#include "elements.h"
#include "mywidget.h"
#include "avalonmatrices.h"

#include <QApplication>
#include <QtGui>
#include <QFileDialog>
#include <QString>
#include <QDebug>

#include <typeinfo>

#include <QtCore>
#include <QPainter>

#include <fstream>
#include <string>
#include <vector>
#include <sstream>

/*////////////////////////////////////////*/
/*Globals defined in globals.h, should all be externed.*/
//This will store the elements read from the following fstream loop.
std::vector<element> elements;
std::vector<step> steps;
std::vector<material> materials;
//Vector to store our nodes into, which will then be read into the elements vector.  To save ram, this vector can be cleared after transfer to elements.
std::vector<node> nodes;
bool globalDebugLoadButtonSectionTags = false;
bool processingComplete = false;
/*////////////////////////////////////////*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);

    //We have created the main window.  We now initalize the values of needed widgets.
    ui->errorTextBrowser->appendPlainText("Please load a file.");
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow(){
    delete ui;
}

//Placeholder error function for when the input file is not set up correctly.
void MainWindow::errorInLoadFileButton(std::string s){
    ui->errorTextBrowser->appendPlainText(QString::fromStdString(s));
    ui->loadFileButton->setEnabled(true);
    ui->progressLabel->setText("No Current Process Running.");
    return;
}

//This loads a user selected file,
void MainWindow::on_loadFileButton_clicked(){
    ui->loadFileButton->setEnabled(false);

    std::fstream infile;
    std::string filename = QFileDialog::getOpenFileName(this, tr("INP file"), qApp->applicationDirPath (),tr("INP File (*.inp)")).toStdString();
    if (filename.empty()){
        ui->loadFileButton->setEnabled(true);
        return;
    }

    processingComplete = false;

    ui->progressLabel->setText("Loading " + QString::fromStdString(filename) + ".");

    //When we are loading a new file we have to wipe the globals.
    wipeGlobals();

    infile.open(filename, std::fstream:: in | std::fstream::out | std::fstream::app);

    std::string tempS;
    std::string line;
    //Current section we're on (example: Node).  Note default is empty.
    std::string section;
    //We will use this string vector to parse the input file by whitespaces when appropriate.
    std::vector<std::string> split;

    //The name of the current element that we are working with (for entering area).
    std::string currentElementName;

    //Same concept for material.
    std::string currentMaterialName;

    //For steps vector.
    size_t currentStepIndex = 0;

    //For user to know what line gave an error (if applicable).
    int lineNumber = 0;

    //For progress bar.
    double totalNumberOfLines = (int)std::count(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>(), '\n');

    //Have to reset fstream because the totalNumberOfLines hits the eof flag.
    infile.clear();
    infile.seekg(0, std::ios::beg);

    //Our main read loop for reading the input files correctly.
    while (std::getline(infile, line)){
        //Regarding comma parse style:
        //This can be countered softly by checking if comma lacks a whitespace following and inserting whitespace if so.  Comment copied from below.
        //Lets do this before I forget.
        //We start at one because there is no point starting at zero.
        for (size_t gon = 1; gon < line.size(); gon++){
            if (line[gon-1] == ',' && line[gon] != ' '){
                line.insert(line.begin()+gon, ' ');
            }
        }
        if (globalDebugLoadButtonSectionTags)
            qDebug() << QString::fromStdString(line);

        //We update the lineNumber counter and the progress bar.  Note that updating progress bar can be expensive, limit occurences in case of huge input file.
        //Oh I'm retarded
        if (totalNumberOfLines > 200){
            if (int(lineNumber)%int(totalNumberOfLines/100)==0)
                ui->progressBar->setValue(int(lineNumber/totalNumberOfLines));
        }
        else
            ui->progressBar->setValue(int(lineNumber/totalNumberOfLines));
        lineNumber++;

        //We begin by clearing unneeded whitespaces from the string (the ones preceeding any non whitespace characters).
        for (; line.size() > 0;){
            //We delete preceeding whitespaces.
            if (line[0] == ' '){
                line.erase(line.begin());
                continue;
            }
            //In the case the first character is no longer a whitespace we break the for loop.
            break;
        }

        //We skip empty lines and comment lines (first two non whitespace chars being **).
        if (line == "" || (line[0] == '*' && line[1] == '*'))
            continue;

        //We are now at the point where we are reading real input.  We set the section if applicable, skip if not.
        //Note that all these lines will start with *.  Save some time here.
        if (line[0] == '*'){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering *";
            //No input follows *NODE on same line.
            //Note: can parameters follow the node keyword?  If so, implement vector akin to stepVector.
            if (beginsWith(uppercase(line), NODE)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering NODE";
                section = NODE;
                continue;
            }

            //Here type and element name (string identifier follow) so we must parse these.
            if (beginsWith(uppercase(line), ELEMENT)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering ELEMENT";
                section = ELEMENT;
                split.clear();
                std::stringstream s(line);
                while (s >> tempS)
                    split.push_back(tempS);
                //At this point we have pushed the strings seperated by whitespace into the vector split.
                //For element, the 3 strings should be *ELEMENT, TYPE, AND ELSET.  We will now apply TYPE and ELSET.
                //These are going to be in index 1 and 2 of split.
                for (size_t sanit = 0; sanit < split.size(); sanit++){
                    removeWhiteSpaces(split[sanit]);
                    removeCommas(split[sanit]);
                }
                //Right now split[1] is TYPE=T2D2.
                //split[2] is ELSET=name.
                //Error checking first.
                if (!beginsWith(uppercase(split[1]), "TYPE=") || !beginsWith(uppercase(split[2]), "ELSET=")){
                    std::string errorString = "Error: TYPE= or ELSET not found in line " + std::to_string(lineNumber) + ".  Line shown below:\n" + line;
                    errorInLoadFileButton(errorString);
                    return;
                }
                //Erase the ELSET from split[2].
                split[2].erase(split[2].begin(), split[2].begin()+6);
                //Check for duplicate names.
                for (size_t e = 0; e < elements.size(); e++){
                    if (elements[e].name == split[2]){
                        std::string errorString = "Error: Duplicate element name found in line " + std::to_string(lineNumber) + ".  Line shown below:\n" + line;
                        errorInLoadFileButton(errorString);
                        return;
                    }
                }
                //No errors.  Create temp element, assign name and type, and push back onto vector.
                element tempE;
                tempE.name = split[2];
                split[1].erase(split[1].begin(), split[1].begin()+5);
                tempE.type = split[1];

                elements.push_back(tempE);
                currentElementName = tempE.name;

                continue;
            }

            //Now we enter the section where we will define material (such as AL) and area (which is in the line below, so we must note the currentElementName).
            if (beginsWith(uppercase(line), SOLID_SECTION)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering SOLID_SECTION";
                //Set the current section.
                section = SOLID_SECTION;

                //Basically do same thing with parsing as element if statement above.
                //Select the correct element, set section (section being the member of the element here), and then set the material name.
                //line is currently: *SOLID SECTION, ELSET=elem1, MATERIAL=AL
                //Split by whitespace into the split vector.
                split.clear();
                std::stringstream s(line);
                //Note here that Solid section is split by a whitespace.  So we have to wing it a little.  Would have been nice to parse by comma.
                //This can be countered softly by checking if comma lacks a whitespace following and inserting whitespace if so.
                while (s >> tempS){
                    if (beginsWith(uppercase(tempS), "SECTION") && split[split.size()-1] == "*SOLID"){
                        split[split.size()-1] = SOLID_SECTION;
                        continue;
                    }
                    split.push_back(tempS);
                }
                //We have our section type, elset, and material as 3 components of split.  We sanitize our strings.
                for (size_t sanit = 0; sanit < split.size(); sanit++){
                    removeWhiteSpaces(split[sanit]);
                    removeCommas(split[sanit]);
                }
                //split[1] == ELSET=elem1 and split[2] == MATERIAL=AL.
                split[1].erase(split[1].begin(), split[1].begin()+6);
                split[2].erase(split[2].begin(), split[2].begin()+9);
                //We now have grabbed the element name and the material name, as well as the cross section type.  Note that white space removed in cross section string.
                currentElementName = split[1];
                int tempI = getCurrentElementIndex(elements, currentElementName);
                //Error checking in the case that the element name on this line is not the name of any element.
                if (tempI == -1){
                    errorInLoadFileButton("Error: Element name from line " + std::to_string(lineNumber) + " not found.  Line shown below:\n" + line + "\nNon existant element name from line was: " + split[1] + ".");
                    return;
                }
                elements[tempI].materialString = split[2];
                elements[tempI].crossSectionType = split[0];
                //We have no pushed back the information on this line into the correct element.
                continue;
            }

            //We will now take care of the STEP type definition.  To be honest I'm not sure what this is doing, will confirm.  NOTE for ctrl f.
            if (beginsWith(uppercase(line), STEP)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering STEP";
                section = STEP;
                //This is not shown in our own examples yet, but I see that keywords can follow the step command.
                //Example: *STEP, PERTURBATION
                //Solution: have vector for step: stepVector, global.
                split.clear();
                std::stringstream s(line);
                while (s >> tempS)
                    split.push_back(tempS);
                //We have read all of the stuff seperated by whitespaces into the split vector.
                //We now sanitize the split vector as normal.
                for (size_t sanit = 0; sanit < split.size(); sanit++){
                    removeWhiteSpaces(split[sanit]);
                    removeCommas(split[sanit]);
                }
                //Split vector is now sanitized.  The first member is the split command, and the following are the parameters.  We push all of this onto the stepVector.
                //I assume that = operator is the fastest way other than pointers.  We have now pushed everything onto the stepVector.
                step tempStep;
                tempStep.parameters = split;

                //Note: It looks like there will always be a keyword following the step command, likely static or dynamic.  Due to this, we
                //will make an exception and grab the next line and push it as a string onto the step vector.
                if (infile.peek() == '*'){
                    //We only proceed if it is a keyword as I think it should always be.
                    lineNumber++;
                    getline(infile, line);
                    split.clear();
                    std::stringstream s(line);
                    while (s >> tempS)
                        split.push_back(tempS);
                    for (size_t sanit = 0; sanit < split.size(); sanit++){
                        removeWhiteSpaces(split[sanit]);
                        removeCommas(split[sanit]);
                    }
                    //We now append the split vector to the end of stepVector.
                    tempStep.parameters.insert(tempStep.parameters.end(), split.begin(), split.end());
                }

                steps.push_back(tempStep);
                currentStepIndex = steps.size() - 1;

                //The static/dynamic/etc parameters on the following line of the *step keyword are now correctly implemented.
                continue;
            }

            //We will now account for *Material keyword section.
            if (beginsWith(uppercase(line), MATERIAL)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering MATERIAL";
                section = MATERIAL;
                split.clear();
                std::stringstream s(line);
                while (s >> tempS)
                    split.push_back(tempS);
                //We now have put the strings seperated by whitespace into split, and while sanitize by removing the commas and white spaces.
                for (size_t sanit = 0; sanit < split.size(); sanit++){
                    removeWhiteSpaces(split[sanit]);
                    removeCommas(split[sanit]);
                }
                //We have now sanitized the input.  Example input is as follows:
                //*MATERIAL and NAME=AL
                //We only need to record the name right now.
                split[1].erase(split[1].begin(), split[1].begin()+5);
                //2nd member of split is now AL in this example.
                //Check for duplicate material name.
                for (size_t oro = 0; oro < materials.size(); oro++){
                    if (materials[oro].name == split[1]){
                        errorInLoadFileButton("Error: Duplicate material name found in line number " + std::to_string(lineNumber) + ".  Line shown below:\n" + line);
                        return;
                    }
                }
                //We have confirmed this is not a duplicate material.  We push it back into the materials vector and set the current material.
                currentMaterialName = split[1];
                material tempMaterial;
                tempMaterial.name = currentMaterialName;
                materials.push_back(tempMaterial);

                //We will be processing the following lines for the respective material in the below area (if chain).
                //This is due to not wanting to have problems with variable amounts of lines for the material.
                continue;
            }

            //Note that this could be under the step section, but is better written to account for more loops like this.
            if (beginsWith(uppercase(line), BOUNDARY)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering BOUNDARY";
                section = BOUNDARY;
                continue;
            }

            //CLOAD (concentrated load keyword).
            if (beginsWith(uppercase(line), CLOAD)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering CLOAD";
                section = CLOAD;
                continue;
            }

            //End keyword -- set up for multiple steps later in quarter.  Placeholder logic here.
            if (beginsWith(uppercase(line), END)){
                if (globalDebugLoadButtonSectionTags)
                    qDebug()<<"Entering END";
                section = END;
                continue;
            }

        }//End if statement looking for * as first char / section identifier.

        /*===========================================================================================*/
        //The section is decided at this point and we are not on a * style line.
        //Error check first.
        if (section.empty()){
            errorInLoadFileButton("Error: Section not defined before moving to post section information at line number: " + std::to_string(lineNumber)+ ".\n"+line);
            return;
        }

        //Node section.
        if (section == NODE){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering NODE2";
            //Example line: 1, 0.0, 0.0
            split.clear();
            std::stringstream s(line);
            while (s >> tempS)
                split.push_back(tempS);
            //Now I should have up to 4 members of split - the name, x, y, and z.  We need to sanitize what we read.
            for (size_t tt = 0; tt < split.size(); tt++){
                removeWhiteSpaces(split[tt]);
                removeCommas(split[tt]);
            }
            long double x, y, z;
            x = std::stod(split[1]);
            y = std::stod(split[2]);
            //Note that we're not always going to get the 3d inputs (in the first example file we have 2d).  So:
            if (split.size() == 4)
                z = std::stod(split[3]);
            else
                z = 0;
            node tempN(x, y ,z);
            tempN.name = split[0];
            //We have created our lovely node, we push it onto the node vector.
            nodes.push_back(tempN);
            continue;
        }

        //Element section.
        if (section == ELEMENT){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering ELEMENT2";
            //Example line: 1, 1, 2.  Element secondaryIdentifier, node 1, node 2.
            split.clear();
            std::stringstream s(line);
            while (s >> tempS)
                split.push_back(tempS);
            //Clean our input.
            for (size_t tt=0; tt < split.size(); tt++){
                removeWhiteSpaces(split[tt]);
                removeCommas(split[tt]);
            }
            //We got here because there was *Element on the previous line.  This means that the last element in the elements vector is the one we're adjusting.
            //Error check first to prevent undefined behavior.
            if (elements.empty()){
                errorInLoadFileButton("Error: Tried to enter element nodes but no element exists yet.  Line number: " + std::to_string(lineNumber)+".\n"+line);
                return;
            }
            //Index of last element.
            size_t tempIndex = getCurrentElementIndex(elements, currentElementName);
            subelement tempSubElement;
            tempSubElement.name = split[0];
            //We have currentElementName.
            //We now loop through all of the nodes connected to the element.
            //Outerloop is looping throughthe listed nodes.
            //Example: 11, 101, 102 (FrameKE.inp).  Starting at 1 not 0 because the first member of split is the subelementName.
            for (size_t i = 1; i < split.size(); i++){
                for (size_t j = 0; j < nodes.size(); j++){
                    //We push back the node once it is found as it has the coords in it, and we move to the next split[i].
                    if (split[i] == nodes[j].name){
                        tempSubElement.nodes.push_back(nodes[j]);
                        break;
                    }
                    //If we've cycled through all of it and haven't found a node with a matching name, there is an error.
                    if (j == nodes.size()-1){
                        errorInLoadFileButton("Error: You are trying to assign a node which does not exist yet to an element.  Line number: " + std::to_string(lineNumber)+".\n"+line);
                        return;
                    }
                }
            }

            //Assuming the tempSubElements.node is not empty, we push it onto the subelements vector of the respect element in the elements vector.
            if (!tempSubElement.nodes.empty()){
                elements[tempIndex].subelements.push_back(tempSubElement);
            }

            //We have now done everything in the Element section.
            continue;
        }

        //Cross-section section (area).
        if (section == SOLID_SECTION){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering SOLID_SECTION2";
            //This will be done once it is implemented in the *Solid... identification above, as I need to know what element I am
            //recording the area for.
            //When I implement the function which uses currentElementName to find the index in elements vector I need to update the
            //elements section to do this instead of using the last element.
            int tempI = getCurrentElementIndex(elements, currentElementName);
            //Right now line is just going to have the area it seems, can update in future though.
            //Sanitize just in case.
            removeWhiteSpaces(line);
            removeCommas(line);
            //Confirm valid input as I don't know what errors stod will throw given incorrect input.
            int numDecimalPoint = 0;
            for (size_t opm = 0; opm < line.size(); opm++){
                if (isdigit(line[opm])){
                    continue;
                }
                else if (line[opm] == '.' && numDecimalPoint < 1){
                    numDecimalPoint++;
                    continue;
                }
                //There is an error at this point.
                errorInLoadFileButton("Error: The area in line number: " + std::to_string(lineNumber)+" is not a correctly formatted double.\n"+line);
                return;
            }
            //We have confirmed that line is a correctly formatted double for stod.
            elements[tempI].area = std::stod(line);
            //We have done everything for this line as currently the only thing on it is cross sectional area. NOTE: May change (on Truss lecture, 1/14 currently).
            continue;
        }

        //Material Section (Elastic).  Include this under the *Material, meaning that I don't have a *Elastic if statement.  Also grab following
        //line for E and v.
        if (section == MATERIAL){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering MATERIAL2";
            //There are two lines that we need to account for, example:
            //*ELASTIC
            //10.0e6, 0.33
            //The difference is going to be the first will have * as first char.
            if (line[0] == '*'){
                //Multiple properties are a possibility.
                int tempIndex = getCurrentMaterialIndex(materials, currentMaterialName);
                split.clear();
                std::stringstream s(line);
                while (s >> tempS)
                    split.push_back(tempS);
                //Clean our input.
                for (size_t tt = 0; tt < split.size(); tt++){
                    removeWhiteSpaces(split[tt]);
                    removeCommas(split[tt]);
                }
                //We push back all the properties onto the correct material.
                for (size_t fma = 0; fma < split.size(); fma++){
                    materials[tempIndex].properties.push_back(split[fma]);
                }
                continue;
            }
            else{
                //In this case we have gotten the line with the modulus and poisson ratio.
                split.clear();
                std::stringstream s(line);
                while (s >> tempS)
                    split.push_back(tempS);
                //Clean our input.
                for (size_t tt = 0; tt < split.size(); tt++){
                    removeWhiteSpaces(split[tt]);
                    removeCommas(split[tt]);
                }
                int tempIndex = getCurrentMaterialIndex(materials, currentMaterialName);
                //So we should have 10.0e6 and 0.33 here in this example (truss example from slides).
                //We need to account for the modulus being a string (because of e).
                materials[tempIndex].youngsModulus = scientificToDouble(split[0]);
                materials[tempIndex].poissonRatio = std::stod(split[1]);

                //We have now updated the material with its modulus and poisson's ratio.
                //We now apply it to the elements that have this material.
                for (size_t lelouch = 0; lelouch < elements.size(); lelouch++){
                    if (elements[lelouch].materialString == materials[tempIndex].name){
                        elements[lelouch].mat = materials[tempIndex];
                    }
                }
                continue;
            }

        }


        //Step section (1., 1.,).
        if (section == STEP){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering STEP2";
            split.clear();
            std::stringstream s(line);
            while (s >> tempS)
                split.push_back(tempS);
            //Clean our input.
            for (size_t tt = 0; tt < split.size(); tt++){
                removeWhiteSpaces(split[tt]);
                removeCommas(split[tt]);
            }
            //We push our step parameters onto the step vector.
            for (size_t fma = 0; fma < split.size(); fma++){
                steps[currentStepIndex].parameters.push_back(split[fma]);
            }
            continue;
        }

        //Boundary Section (Node number, DOF.begin(), DOF.end(), ?
        if (section == BOUNDARY){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering BOUNDARY2";
            split.clear();
            std::stringstream s(line);
            while (s >> tempS)
                split.push_back(tempS);
            //Clean our input.
            for (size_t tt = 0; tt < split.size(); tt++){
                removeWhiteSpaces(split[tt]);
                removeCommas(split[tt]);
            }
            //This stuff is in split:
            //**Node, dofmin, dofmax, set dof in range to last value
            //1, 1, 6, 0.0
            //split[0]is the node name, split[1] is the dofmin, split[2] is the dofmax, split[3] is the dofvalue.
            //So we are interested in looping through the elements[i].subelements.nodes[j].name, checking for a name match, and
            //if so, we change that elements[i].subelements.nodes[j].degreesOfFreedom[k] for k=0:5 in this example (-1 as starts at 0) and
            //make each equal to 0.
            //rdu
            for (size_t r = 0; r < elements.size(); r++){
                for (size_t d = 0; d < elements[r].subelements.size(); d++){
                    for (size_t u = 0; u < elements[r].subelements[d].nodes.size(); u++){
                        if (elements[r].subelements[d].nodes[u].name == split[0]){
                            for (double kai = std::stod(split[1]); kai <= std::stod(split[2]); kai++){
                                degreeOfFreedom tempDegree;
                                tempDegree.identifier = kai;
                                tempDegree.value = std::stoi(split[2]);
                                elements[r].subelements[d].nodes[u].degreesOfFreedom.push_back(tempDegree);
                            }
                        }
                    }
                }
            }
            continue;
        }

        //Cload section (Node Number, force direction.  guess 2 is in y, magnitude)  Can have more parameters on the same line.
        if (section == CLOAD){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering CLOAD2";
            split.clear();
            std::stringstream s(line);
            while (s >> tempS)
                split.push_back(tempS);
            //Clean our input.
            for (size_t tt = 0; tt < split.size(); tt++){
                removeWhiteSpaces(split[tt]);
                removeCommas(split[tt]);
            }
            //The following stuff is in split.  Node number, axis, and magnitude.
            //We want to have angle so that this is simple to extend in future so need utility function.
            //2, 2, -9000.
            //split[0] is node name.  split[1] is node force axis.  split[2] is node magnitude of force.
            force tempF;
            tempF.direction = axisToDirectionVector(std::stod(split[1]));
            tempF.magnitude = std::stod(split[2]);
            for (size_t r = 0; r < elements.size(); r++){
                for (size_t d = 0; d < elements[r].subelements.size(); d++){
                    for (size_t u = 0; u < elements[r].subelements[d].nodes.size(); u++){
                        if (elements[r].subelements[d].nodes[u].name == split[0]){
                            elements[r].subelements[d].nodes[u].forces.push_back(tempF);
                        }
                    }
                }
            }
            //The cload on this line has been applied to the nodes.
            continue;
        }

        //End Step (Note: Can possibly have multiple parameters for end).
        if (section == END){
            if (globalDebugLoadButtonSectionTags)
                qDebug()<<"Entering END2";
            continue;
        }
    }

    ui->progressBar->setValue(100);
    infile.close();

    //We have completed reading the file.  Now we can draw stuff to the widget.
    //myWidget.show();



    QString tempQS = QString::fromStdString(filename) +" loaded with no errors.";
    ui->errorTextBrowser->appendPlainText(tempQS);
    ui->loadFileButton->setEnabled(true);
    ui->progressLabel->setText("No Current Process Running.");
}

/*//element
 * std::vector<subelement> subelements;
    std::string type;
    std::string name;
    std::string materialString;
    std::string crossSectionType;
    long double area;
    material mat;

    //material
    std::string name;
    long double youngsModulus;
    long double poissonRatio;
    std::vector<std::string> properties;

    //subelement
    std::string name;
    std::vector<node> nodes;

    //node
    std::string name;
    long double x;
    long double y;
    long double z;
    std::vector<degreeOfFreedom>degreesOfFreedom;
    std::vector<force> forces;

    //force
    directionVector direction;
    long double magnitude;

    //direction vector
    long double x;
    long double y;
    long double z;

    //degreeOfFreedom
    int identifier;
    int value;
    */

//Prints information about the elements so we can confirm that the stuff is stored correctly.
void MainWindow::on_displayElementsButton_clicked(){
    ui->displayElementsButton->setEnabled(false);
    ui->progressLabel->setText("Displaying elements in memory.");
    for (size_t i = 0; i < elements.size(); i++){
        if (elements.size() > 200){
            if (i%(int)(elements.size()/100)==0)
                ui->progressBar->setValue(int(i/elements.size()));
        }
        else
            ui->progressBar->setValue(int(i/elements.size()));

        ui->errorTextBrowser->appendPlainText("");
        ui->errorTextBrowser->appendPlainText("Element Number: " + QString::fromStdString(std::to_string(i)));

        QString tempS;

        /*Element Stuff============================================================================================*/
        tempS = "Element Name: " + QString::fromStdString(elements[i].name);
        ui->errorTextBrowser->appendPlainText(tempS);
        tempS = "Type: " + QString::fromStdString(elements[i].type);
        ui->errorTextBrowser->appendPlainText(tempS);
        tempS = "Cross Section Type and Area: " + QString::fromStdString(elements[i].crossSectionType) + ", " + QString::fromStdString(std::to_string(elements[i].area));
        ui->errorTextBrowser->appendPlainText(tempS);
        tempS = "Number of subelements: " + QString::fromStdString(std::to_string(elements[i].subelements.size()));
        ui->errorTextBrowser->appendPlainText(tempS);
        /*Material Stuff============================================================================================*/
        tempS = "Material: " + QString::fromStdString(elements[i].mat.name);
        ui->errorTextBrowser->appendPlainText(tempS);
        tempS = "youngsModulus: " + QString::fromStdString(std::to_string(elements[i].mat.youngsModulus));
        ui->errorTextBrowser->appendPlainText(tempS);
        tempS = "poissonRatio: " + QString::fromStdString(std::to_string(elements[i].mat.poissonRatio));
        ui->errorTextBrowser->appendPlainText(tempS);
        for (size_t j = 0; j < elements[i].mat.properties.size(); j++){
            std::string tempPropName = elements[i].mat.properties[j];
            tempS = "Material property " + QString::fromStdString(std::to_string(j)) + ": " + QString::fromStdString(tempPropName);
            ui->errorTextBrowser->appendPlainText(tempS);
        }
        /*Subelement Stuff============================================================================================*/
        //name and nodes vector.
        for (size_t j = 0; j < elements[i].subelements.size(); j++){
            tempS = "subelement: " + QString::fromStdString(std::to_string(j));
            ui->errorTextBrowser->appendPlainText(tempS);
            tempS = "subelement name: " + QString::fromStdString(elements[i].subelements[j].name);
            ui->errorTextBrowser->appendPlainText(tempS);
            tempS = "Number of nodes: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes.size()));
            ui->errorTextBrowser->appendPlainText(tempS);

            /*Node Stuff============================================================================================*/
            /*std::string name;
            long double x;
            long double y;
            long double z;
            std::vector<degreeOfFreedom>degreesOfFreedom;
            std::vector<force> forces;*/
            for (size_t k = 0; k < elements[i].subelements[j].nodes.size(); k++){
                tempS = "node name: ";
                tempS += QString::fromStdString(elements[i].subelements[j].nodes[k].name);
                tempS += ", x: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].x));
                tempS += ", y: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].y));
                tempS += ", z: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].z));
                tempS += ", number of notable degreesOfFreedom: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].degreesOfFreedom.size()));
                tempS += ", number of notable forces:  " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].forces.size()));
                ui->errorTextBrowser->appendPlainText(tempS);
                //This bool is set to complete at the end of the void MainWindow::on_processButton_clicked() function.
                if (processingComplete){
                    tempS = "node translation code numbers (note: only created after processing)";
                    tempS += ", codeX: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].codeX));
                    tempS += ", codeY: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].codeY));
                    tempS += ", codeZ: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].codeZ));
                    ui->errorTextBrowser->appendPlainText(tempS);
                }



                /*degreeOfFreedom, force, directionVector level=========================================================*/
                /*//force
                directionVector direction;
                long double magnitude;

                //direction vector
                long double x;
                long double y;
                long double z;

                //degreeOfFreedom
                int identifier;
                int value;
                */
                //ijk xyz
                //force
                for (size_t x = 0; x < elements[i].subelements[j].nodes[k].forces.size(); x++){
                    tempS = "force " + QString::fromStdString(std::to_string(x));
                    tempS += "- magnitude: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].forces[x].magnitude));
                    tempS += ", direction <x,y,z>: <";
                    tempS += QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].forces[x].direction.x)) + ", ";
                    tempS += QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].forces[x].direction.y)) + ", ";
                    tempS += QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].forces[x].direction.z)) + ">";
                    ui->errorTextBrowser->appendPlainText(tempS);
                }

                //degreeOfFreedom
                for (size_t x = 0; x < elements[i].subelements[j].nodes[k].degreesOfFreedom.size(); x++){
                    tempS = "degree of freedom number " + QString::fromStdString(std::to_string(x));
                    tempS += "- identifier: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].degreesOfFreedom[x].identifier));
                    tempS += ", value: " + QString::fromStdString(std::to_string(elements[i].subelements[j].nodes[k].degreesOfFreedom[x].value));
                    ui->errorTextBrowser->appendPlainText(tempS);
                }

            }



        }


    }
    ui->errorTextBrowser->appendPlainText("");
    ui->progressLabel->setText("No Current Process Running.");
    ui->displayElementsButton->setEnabled(true);
    ui->progressBar->setValue(100);
}

//Using to test the matrix functions right now.  Also if they work with qDebug or not.
void MainWindow::on_processButton_clicked(){
    //mTest();
    ui->processButton->setEnabled(false);
    ui->errorTextBrowser->appendPlainText("File processing started.");
    assignCodeNumbers(elements, nodes);

    //Check if an error was thrown.
    //Note: Make it so that NOERROR is an invalid name.
    if (assignSubelementAngles(elements).name != "NOERROR"){
        //Can change name later.
        std::string tempS = "Error in assignSubelementsAngles function with subelement with name ";
        tempS += assignSubelementAngles(elements).name;
        tempS += ": number of attached nodes does not equal two.";
        errorInLoadFileButton(tempS);
        ui->errorTextBrowser->appendPlainText("Processing exited without completing.");
        ui->processButton->setEnabled(true);
    }


    ui->errorTextBrowser->appendPlainText("File processing completed.");
    ui->processButton->setEnabled(true);
    processingComplete = true;
}
