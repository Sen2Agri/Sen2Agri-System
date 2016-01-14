#ifndef PHENONDVIHANDLER_HPP
#define PHENONDVIHANDLER_HPP

#include "processorhandler.hpp"

class PhenoNdviHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;
};

#endif // PHENONDVIHANDLER_HPP

