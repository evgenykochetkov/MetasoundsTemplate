/*
NOTE: if you are using this file as a template, make sure to
replace all occurrences of 'AnnotatedTutorial' and 'MyCustomNodes'
*/

/*
Sources:
 - https://dev.epicgames.com/community/learning/tutorials/ry7p/creating-metasound
 - TODO: https://dev.epicgames.com/community/learning/tutorials/KJWk/writing-a-pitch-shift-metasound-node
Both are a bit outdated.
Actual code heavily borrowed from https://github.com/alexirae/unreal-audio-dsp-template-UE5/
*/

/*
Writing a new MetaSound node requires only a single private .cpp file.

There are two classes which need to be defined:
 - An operator class deriving from TExecutableOperator
 - A node class deriving from FNodeFacade
*/


// For METASOUND_PARAM and METASOUND_GET_PARAM_* macros (more about them below)
#include "MetasoundParamHelper.h"

// This is copied from alexirae/unreal-audio-dsp-template-UE5.
// The alternative is a whole bunch of #includes, I guess
#include "MetasoundEnumRegistrationMacro.h"

// Our own DSP utility
#include "MyDSP/Volume.h"


/*
The MetaSound standard node library is localized for all the languages supported by the engine.
It uses the LOCTEXT system to generate text which needs localization.

You’ll need to make sure your localization namespace is unique per node
and that you take into account localization keys when defining your node names and tooltips.

Immediately following the header includes in your .cpp file
should be a #define for a LOCTEXT namespace unique to your node
*/
#define LOCTEXT_NAMESPACE "MyCustomNodes_AnnotatedTutorialNode"
/* At the very end of your .cpp file, the LOCTEXT_NAMESPACE needs to be undefined */


namespace MyCustomNodes {
using namespace Metasound;

#pragma region Parameter Names

/*
To avoid mistakes, we've evolved our node creating methodology to declare
the parameters, their variable name, their friendly editor-name, and tooltips,
at the top of the node definition files using a useful macro, "METASOUND_PARAM".

We also utilize node-specific namespaces to avoid collisions of variable names between nodes we make. 

We will reference these defined parameters in the rest of the node implementation, detailed below, as follows:
 - When retrieving the name and tooltip:      METASOUND_GET_PARAM_NAME_AND_METADATA(InputAValue)
 - When retrieving just the parameter name:   METASOUND_GET_PARAM_NAME(InputAValue)
*/
namespace AnnotatedTutorialNodeParameterNames {
  // Input params
  METASOUND_PARAM(InParamNameAudioInput, "In", "Audio input.")
  METASOUND_PARAM(InParamNameAmplitude, "Amplitude", "The amount of amplitude to apply to the input signal.")

  // Output params
  METASOUND_PARAM(OutParamNameAudio, "Out", "Audio output.")

  /*
  We typically put internal, namespaced constants right after the parameter list.
  For example, some hard-coded parameters that should be tweakable only by programmers.
  */
}

#pragma endregion

#pragma region Operator

/*
This class defines the way your node is described, created and executed.
It’s also the object which is instantiated at runtime in a MetaSound graph.

The operator describes itself via the function GetNodeInfo(),
creates itself by the function CreateOperator()
and is executed in the function Execute(). 
*/
class FAnnotatedTutorialOperator : public TExecutableOperator<FAnnotatedTutorialOperator> {
  /*
  You will need your class to store read references to all inputs
  and write references to all outputs your node will use.

  The necessary reference classes use the naming convention F[Type]ReadRef and F[Type]WriteRef,
  and exist for the primitive types int, float, string, and bool,
  as well as the MetaSound-specific types Trigger, Audio Buffer, Audio Modulation Parameter, Time, and Wave Asset.
  For types like Audio Buffer and Trigger, you may need to include additional header files.
  */
  FAudioBufferReadRef AudioInput;
  FFloatReadRef Amplitude;
  FAudioBufferWriteRef AudioOutput;
  
  MyDSP::FVolume VolumeDSPProcessor;

public:
  /*
  The Constructor will need a const FOperatorSettings &
  and const read references to the node’s input variables as input parameters to the function.

  The constructor will need to create write references to the node’s output parameters,
  and initialize both the input and output parameters to reasonable values.

  When working with an output node that is a Trigger or Audio Buffer type,
  the output node needs to be initialized with FTriggerWriteRef::CreateNew(InSettings)
  or FAudioBufferWriteRef::CreateNew(InSettings), respectively.
  */
  FAnnotatedTutorialOperator(const FOperatorSettings &InSettings,
                             const FAudioBufferReadRef &InAudioInput,
                             const FFloatReadRef &InAmplitude)
    : AudioInput(InAudioInput),
      Amplitude(InAmplitude),
      AudioOutput(FAudioBufferWriteRef::CreateNew(InSettings)) {
    // Here we could initialize some custom DSP stuff
  }

  /*
  This is a function you need to provide for your node
  which retrieves necessary metadata about the MetaSound node.

  It provides the class name including the MetaSound namespace,
  which show up as subcategories in the MetaSound editor.

  It also provides other information such as the version number, the localizable display name,
  a localizable description, the node author, and a prompt to tell the user if it’s missing
  (i.e. somebody has a MetaSound graph that they are loading, but without your plugin).

  Most importantly, this function returns your default vertex interface definition,
  which is essentially the objects and types of the Node’s inputs and outputs (i.e. vertices).
  */
  static const FNodeClassMetadata &GetNodeInfo() {
    auto InitNodeInfo = []() -> FNodeClassMetadata {
      FNodeClassMetadata Info;

      Info.ClassName = {TEXT("UE"), TEXT("Annotated Tutorial"), TEXT("Audio")};
      Info.MajorVersion = 1;
      Info.MinorVersion = 0;
      Info.DisplayName = LOCTEXT("MyCustomNodes_AnnotatedTutorialDisplayName", "My Annotated Tutorial Node");
      Info.Description = LOCTEXT("MyCustomNodes_AnnotatedTutorialNodeDescription",
                                 "A basic node with annotaded source code.");
      Info.Author = "Put your name here!.";
      Info.PromptIfMissing = PluginNodeMissingPrompt;
      Info.DefaultInterface = GetVertexInterface();
      Info.CategoryHierarchy = {LOCTEXT("MyCustomNodes_AnnotatedTutorialNodeCategory", "My Custom Category")};

      return Info;
    };

    static const FNodeClassMetadata Info = InitNodeInfo();

    return Info;  
  }

  /*
  In graph terminology, our MetaSound “parameters” (or pins) are known as vertices.
  To define a MetaSound node, the graph builder needs to understand the vertices to build the node.

  Because it’s referenced in a few places in the node definition,
  it often helps to define a static local helper function that constructs a FVertexInterface object
  */
  static const FVertexInterface &GetVertexInterface() {
    using namespace AnnotatedTutorialNodeParameterNames;

    static TInputDataVertex<FAudioBuffer> AudioInputVertex(
        METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAudioInput));

    static TInputDataVertex<float> AmplitudeInputVertex(
        METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAmplitude),
        1.0f);

    static TOutputDataVertex<FAudioBuffer> AudioOutputVertex(
        METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAudio));

    static const FVertexInterface Interface(
        FInputVertexInterface(AudioInputVertex, AmplitudeInputVertex),
        FOutputVertexInterface(AudioOutputVertex)
        );

    return Interface;
  }

  static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams &InParams,
                                                         FBuildResults &OutResults) {
    using namespace AnnotatedTutorialNodeParameterNames;

    FAudioBufferReadRef AudioIn =
      InParams.InputData.GetOrConstructDataReadReference<FAudioBuffer>(
          METASOUND_GET_PARAM_NAME(InParamNameAudioInput),
          InParams.OperatorSettings);

    FFloatReadRef InAmplitude =
      InParams.InputData.GetOrCreateDefaultDataReadReference<float>(
         METASOUND_GET_PARAM_NAME(InParamNameAmplitude),
         InParams.OperatorSettings);

    return MakeUnique<FAnnotatedTutorialOperator>(
        InParams.OperatorSettings,
        AudioIn,
        InAmplitude);
  }
  
  virtual void BindInputs(FInputVertexInterfaceData &InOutVertexData) override {
    using namespace AnnotatedTutorialNodeParameterNames;

    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameAudioInput), AudioInput);
    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameAmplitude), Amplitude);
  }
  
  virtual void BindOutputs(FOutputVertexInterfaceData &InOutVertexData) override {
    using namespace AnnotatedTutorialNodeParameterNames;

    InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAudio), AudioOutput);
  }

  /*
  This is where the MetaSound’s primary functionality goes.
  In other words, this is where you should place any math, digital signal processing,
  or other operations that you want the node to perform each frame.

  The implementation of this function can vary wildly in complexity
  depending on the intended functionality of the node

  It’s worth noting that the execute function will be called very frequently
  in the MetaSound graph once the Node is placed.
  As such, it’s important to avoid slow or blocking functions here.
  Otherwise, you run the risk of having performance problems
  when your node is used within a MetaSound, which can lead to irritating pops and clicks.

  Some best practices include utilizing buffer-optimized code,
  such as those you can find in the file BufferVectorOperations.cpp,
  instead of any sample-by-sample loops.

  In addition, in nodes where redoing calculations can be expensive,
  it’s often worth storing the last previous value(s) of the input parameters
  to verify if the output needs to change.  
  */
  void Execute() {
    const float *InputAudio = AudioInput->GetData();
    float *OutputAudio = AudioOutput->GetData();

    const int32 NumSamples = AudioInput->Num();

    VolumeDSPProcessor.SetAmplitude(*Amplitude);
    VolumeDSPProcessor.ProcessAudioBuffer(InputAudio, OutputAudio, NumSamples);
  }
};

#pragma endregion

#pragma region Node

class FAnnotatedTutorialNode : public FNodeFacade {
public:
  // Constructor used by the Metasound Frontend.
  FAnnotatedTutorialNode(const FNodeInitData &InitData)
      : FNodeFacade(InitData.InstanceName,
                               InitData.InstanceID,
                               Metasound::TFacadeOperatorClass<FAnnotatedTutorialOperator>()) {}
};

METASOUND_REGISTER_NODE(FAnnotatedTutorialNode)

#pragma endregion

}

#undef LOCTEXT_NAMESPACE
