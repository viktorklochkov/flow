//
// Created by Lukas Kreis on 12.06.17.
//

#ifndef FLOW_TASK_H
#define FLOW_TASK_H


class Task {
 public:
  void Run();
 private:
  virtual void Init() = 0;
  virtual void Exec() = 0;
  virtual void Finish() = 0;
  virtual void Cleanup() = 0;
};


#endif //FLOW_TASK_H
